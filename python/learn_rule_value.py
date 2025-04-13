import torch
import torch.nn as nn
import numpy as np
import json


class RuleEncoder:
    """Encodes rule JSON into tensor representation"""
    def __init__(self, n_states, n_dims):
        self.n_states = n_states
        self.max_nbrs = int(3**n_dims - 1)

    def encode(self, rule_json):
        # Create empty tensors
        rule_tensor = torch.zeros(self.n_states, self.n_states, self.max_nbrs)
        live_states = torch.zeros(self.n_states)

        # Fill in live states
        for state in rule_json["live_states"]:
            live_states[state] = 1.0

        # Fill in rule table
        for i, row in enumerate(rule_json["table"]):
            for j, cell in enumerate(row):
                if cell == "A":
                    # "A" means all neighbor counts are included
                    rule_tensor[i, j, :] = 1.0
                elif cell == "C":
                    # "C" means complement of all other chosen counts in this row
                    # First, find all counts already used in this row
                    used_counts = set()
                    for k, other_cell in enumerate(row):
                        if k != j and other_cell not in ["A", "C", "-"]:
                            counts = [int(val) for val in other_cell.split(",") if val.strip()]
                            used_counts.update(counts)

                    # Set all unused counts to 1 (adjusting for 0-based indexing)
                    for n in range(1, self.max_nbrs + 1):
                        if n not in used_counts:
                            rule_tensor[i, j, n - 1] = 1.0
                elif cell == "-":
                    # "-" means no neighbor counts are included
                    pass  # tensor already initialized with zeros
                else:
                    # Parse comma-separated values
                    values = cell.split(",")
                    for val in values:
                        if val.strip():  # Skip empty values
                            rule_tensor[i, j, int(val) - 1] = 1.0  # Adjust for 0-based indexing

        return rule_tensor, live_states


class RuleValueMLP(nn.Module):
    """Neural network to estimate rule value using an MLP with configurable hidden layers"""
    def __init__(self, n_states, max_nbrs, hidden_layers=None):
        super().__init__()

        # Default hidden layers if none provided
        if hidden_layers is None:
            hidden_layers = [256, 128, 64]

        # Calculate input size
        rule_size = n_states * n_states * max_nbrs  # Flattened rule tensor
        live_states_size = n_states  # Live states indicator
        input_size = rule_size + live_states_size

        # Build MLP with configurable hidden layers
        layers = []
        prev_size = input_size

        for hidden_size in hidden_layers:
            layers.append(nn.Linear(prev_size, hidden_size))
            layers.append(nn.ReLU())
            prev_size = hidden_size

        # Output layer
        layers.append(nn.Linear(prev_size, 1))

        # Create sequential model
        self.network = nn.Sequential(*layers)

    def forward(self, rule_tensor, live_states):
        batch_size = rule_tensor.shape[0]

        # Flatten the rule tensor
        flat_rule = rule_tensor.view(batch_size, -1)

        # Concatenate with live states
        combined = torch.cat([flat_rule, live_states], dim=1)

        # Predict value
        value = self.network(combined)

        return value


class RuleValueTrainer:
    """Trains the value network using rule-reward pairs"""
    def __init__(self, n_states, n_dims, hidden_layers=None, learning_rate=0.001):
        self.encoder = RuleEncoder(n_states, n_dims)
        self.n_states = n_states
        self.n_dims = n_dims
        self.max_nbrs = int(3**n_dims - 1)

        # Initialize model
        self.model = RuleValueMLP(n_states, self.max_nbrs, hidden_layers)
        self.optimizer = torch.optim.Adam(self.model.parameters(), lr=learning_rate)
        self.loss_fn = nn.MSELoss()

    def train(self, rule_json, reward, epochs=1):
        rule_tensor, live_states = self.encoder.encode(rule_json)
        rule_tensor = rule_tensor.unsqueeze(0)  # Add batch dimension
        live_states = live_states.unsqueeze(0)
        target = torch.tensor([[reward]], dtype=torch.float32)

        losses = []
        for _ in range(epochs):
            self.optimizer.zero_grad()
            value = self.model(rule_tensor, live_states)
            loss = self.loss_fn(value, target)
            loss.backward()
            self.optimizer.step()
            losses.append(loss.item())

        return np.mean(losses)

    def train_batch(self, rule_jsons, rewards, batch_size=32, epochs=1):
        n_samples = len(rule_jsons)
        indices = list(range(n_samples))

        total_loss = 0
        for _ in range(epochs):
            np.random.shuffle(indices)

            for i in range(0, n_samples, batch_size):
                batch_indices = indices[i:i+batch_size]

                # Prepare batch
                rule_tensors = []
                live_states_list = []
                batch_rewards = []

                for idx in batch_indices:
                    rule_tensor, live_states = self.encoder.encode(rule_jsons[idx])
                    rule_tensors.append(rule_tensor)
                    live_states_list.append(live_states)
                    batch_rewards.append(rewards[idx])

                rule_tensors = torch.stack(rule_tensors)
                live_states_list = torch.stack(live_states_list)
                batch_rewards = torch.tensor(batch_rewards, dtype=torch.float32).unsqueeze(1)

                # Train on batch
                self.optimizer.zero_grad()
                values = self.model(rule_tensors, live_states_list)
                loss = self.loss_fn(values, batch_rewards)
                loss.backward()
                self.optimizer.step()

                total_loss += loss.item() * len(batch_indices)

        return total_loss / (n_samples * epochs)

    def predict(self, rule_json):
        with torch.no_grad():
            rule_tensor, live_states = self.encoder.encode(rule_json)
            rule_tensor = rule_tensor.unsqueeze(0)
            live_states = live_states.unsqueeze(0)
            value = self.model(rule_tensor, live_states)
            return value.item()

    def save(self, path):
        """Save model and configuration"""
        torch.save({
            'model_state_dict': self.model.state_dict(),
            'optimizer_state_dict': self.optimizer.state_dict(),
            'n_states': self.n_states,
            'n_dims': self.n_dims,
        }, path)

    def load(self, path):
        """Load model and configuration"""
        checkpoint = torch.load(path)

        # Ensure model matches saved configuration
        if self.n_states != checkpoint['n_states'] or self.n_dims != checkpoint['n_dims']:
            raise ValueError(f"Model mismatch: saved n_states={checkpoint['n_states']}, n_dims={checkpoint['n_dims']} "
                             f"but current n_states={self.n_states}, n_dims={self.n_dims}")

        self.model.load_state_dict(checkpoint['model_state_dict'])
        self.optimizer.load_state_dict(checkpoint['optimizer_state_dict'])