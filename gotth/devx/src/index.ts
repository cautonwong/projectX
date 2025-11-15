#!/usr/bin/env node
import { Command } from "commander";
import { initCommand } from "./commands/init";
import { newServiceCommand } from "./commands/new-service";
import { newFrontendCommand } from "./commands/new-frontend";
import { dbMigrateCommand } from "./commands/db-migrate";
import { runCommand } from "./commands/run";
import { pluginAddCommand } from "./commands/plugin-add";
import { doctorCommand } from "./commands/doctor";
import { upgradeCommand } from "./commands/upgrade";

const program = new Command();

program
  .name("devx")
  .description("Fullstack multi-framework boilerplate CLI")
  .version("1.0.0");

program.addCommand(initCommand);
program.addCommand(newServiceCommand);
program.addCommand(newFrontendCommand);
program.addCommand(dbMigrateCommand);
program.addCommand(runCommand);
program.addCommand(pluginAddCommand);
program.addCommand(doctorCommand);
program.addCommand(upgradeCommand);

program.parse();
