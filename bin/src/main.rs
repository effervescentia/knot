mod args;
mod build;
mod check;
mod format;
mod reporter;

use args::{Args, Command};
use clap::Parser;
use kore::color::Highlight;

fn main() {
    let args = Args::parse();
    let command = format!("knot:{}", args.command);

    eprintln!("{} - running \u{1f680}\n", command.focus());

    let result = match args.command {
        Command::Format { glob, root_dir } => format::command(format::Args { glob, root_dir }),

        Command::Check {
            entry,
            root_dir,
            source_dir,
        } => check::command(check::Args {
            entry,
            root_dir,
            source_dir,
        }),

        Command::Build {
            target,
            entry,
            root_dir,
            source_dir,
            out_dir,
        } => build::command(build::Args {
            target,
            entry,
            root_dir,
            source_dir,
            out_dir,
        }),
    };

    match result {
        Ok(()) => {
            eprintln!("{} - passed \u{2705}", command.focus());
        }
        Err(errs) => {
            reporter::eprint_report(&errs);

            eprintln!("{} - failed \u{274c}", command.focus());

            std::process::exit(exitcode::DATAERR);
        }
    }
}
