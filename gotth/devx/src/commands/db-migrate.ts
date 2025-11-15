import { Command } from "commander";
import { exec } from "../utils/exec";

export const dbMigrateCommand = new Command("db migrate")
  .description("Run database migrations")
  .action(async () => {
    await exec("sqlc generate", [], "backend");
    console.log("✅ Database migration done");
  });
