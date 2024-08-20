mod args;
mod build;
mod check;
mod config;
mod format;
mod log;
mod path;

use args::{Args, Command};
use clap::Parser;
use command::Phase;
use kore::color::Highlight;

fn main() {
    let args = Args::parse();
    let command = format!("knot:{}", args.command);

    eprintln!("{} - running \u{1f680}\n", command.focus());

    let result = match args.command {
        Command::Format {
            ref root_dir,
            ref glob,
        } => format::command(format::Args { root_dir, glob }),

        Command::Check {
            ref root_dir,
            ref source_dir,
            ref entry,
        } => check::command(check::Args {
            root_dir,
            source_dir,
            entry,
        }),

        Command::Build {
            target,
            ref out_dir,
            ref root_dir,
            ref source_dir,
            ref entry,
        } => build::command(build::Args {
            target,
            out_dir,
            entry,
            root_dir,
            source_dir,
        }),
    };

    match result {
        Ok(()) => {
            eprintln!("{}{} - passed \u{2705}", Phase::Result, command.focus());
        }
        Err(report) => {
            eprint!("{}{}", Phase::Report, report);

            eprintln!("{}{} - failed \u{274c}", Phase::Result, command.focus());

            std::process::exit(exitcode::DATAERR);
        }
    }
}
