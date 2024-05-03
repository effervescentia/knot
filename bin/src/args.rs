use clap::{Parser, Subcommand, ValueEnum};
use std::{fmt::Display, path::PathBuf};

#[derive(Debug, Parser)]
#[command(name = "knot")]
#[command(version = "3.0")]
#[command(about, long_about = None)]
pub struct Args {
    #[command(subcommand)]
    pub command: Command,
}

#[derive(Copy, Clone, Debug, Eq, Ord, PartialEq, PartialOrd, ValueEnum)]
pub enum Target {
    JavaScript,
}

#[derive(Clone, Debug, Subcommand)]
pub enum Command {
    Format {
        glob: String,

        /* options */
        #[arg(short, long, default_value = "src")]
        root_dir: PathBuf,
    },

    Check {
        /* options */
        #[arg(short, long, default_value = "main.kn")]
        entry: PathBuf,

        #[arg(short, long, default_value = ".")]
        root_dir: PathBuf,

        #[arg(short, long, default_value = "src")]
        source_dir: PathBuf,
    },

    Build {
        target: Target,

        /* options */
        #[arg(short, long, default_value = "main.kn")]
        entry: PathBuf,

        #[arg(short, long, default_value = ".")]
        root_dir: PathBuf,

        #[arg(short, long, default_value = "src")]
        source_dir: PathBuf,

        #[arg(short, long, default_value = "build")]
        out_dir: PathBuf,
    },
}

impl Display for Command {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        f.write_str(match self {
            Self::Format { .. } => "format",
            Self::Check { .. } => "check",
            Self::Build { .. } => "build",
        })
    }
}
