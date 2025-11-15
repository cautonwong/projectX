import inquirer from "inquirer";

export async function ask(questions: any[]) {
  return inquirer.prompt(questions);
}
