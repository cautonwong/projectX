import { Command } from "commander";
import concurrently from "concurrently";

export const runCommand = new Command("run")
  .description("Run backend and frontend concurrently")
  .action(async () => {
    await concurrently([
      { command: "go run backend/main.go", name: "backend" },
      { command: "flutter run -d chrome", name: "frontend" },
    ]);
  });
