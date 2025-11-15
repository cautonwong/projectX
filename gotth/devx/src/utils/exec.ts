import { spawn } from "child_process";

export async function exec(cmd: string, args: string[] = [], cwd?: string) {
  return new Promise<void>((resolve, reject) => {
    const child = spawn(cmd, args, { stdio: "inherit", cwd, shell: true });
    child.on("close", code => {
      if (code !== 0) reject(new Error(`Command failed: ${cmd}`));
      else resolve();
    });
  });
}
