import os
import subprocess

import tqdm

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GOL3D_EXEC = os.path.join(ROOT_DIR, 'cmake-build-release', 'gol3d')


def process_rules(
        rule_files: list[str],
        save_dir: str) -> None:
    """Run simulations for a collection of rule files.

    Args:
    rule_files: Paths to rule file JSONs.
    save_dir: Dir to save run stats to.

    Returns: None
    """
    for rule_file in tqdm.tqdm(rule_files):
        save_file = os.path.join(save_dir, os.path.basename(rule_file))
        run_cmd = f'{GOL3D_EXEC} {rule_file} {save_file}'
        subprocess.run(run_cmd, shell=True)
    return


if __name__ == '__main__':
    import sys
    rule_dir = sys.argv[1]
    if len(sys.argv) > 2:
        save_dir = sys.argv[2]
    else:
        save_dir = rule_dir.replace('rules', 'results')

    rule_files = [
        os.path.join(rule_dir, f) for f in os.listdir(rule_dir)
        if f.endswith('.json')
    ]
    process_rules(rule_files, save_dir)
