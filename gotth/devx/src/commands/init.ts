import { Command } from "commander";
import { ask } from "../utils/prompt";
import { exec } from "../utils/exec";
import fs from "fs";

export const initCommand = new Command("init")
  .description("Initialize new project")
  .action(async () => {
    const { projectName, backend, frontend } = await ask([
      { name: "projectName", message: "Project name:", default: "myapp" },
      { type: "list", name: "backend", message: "Choose backend:", choices: ["go", "rust", "node", "uv"] },
      { type: "list", name: "frontend", message: "Choose frontend:", choices: ["flutter", "tauri", "qml"] },
    ]);

    console.log(`🚀 Creating ${projectName} with ${backend} backend and ${frontend} frontend...`);

    fs.mkdirSync(projectName);
    process.chdir(projectName);
    fs.mkdirSync("backend");
    fs.mkdirSync("frontend");

    await exec(`cp -r ../tooling/devx/src/templates/service-${backend}/ backend/`);
    await exec(`cp -r ../tooling/devx/src/templates/frontend-${frontend}/ frontend/`);
    console.log("✅ Project initialized successfully!");
  });
