import { Command } from "commander";
import { exec } from "../utils/exec";

export const pluginAddCommand = new Command("plugin add")
  .argument("<name>", "Plugin name")
  .action(async (name) => {
    console.log(`🔌 Installing plugin: ${name}`);
    await exec(`git clone https://github.com/yourorg/${name}.git plugins/${name}`);
    console.log("✅ Plugin added successfully!");
  });
