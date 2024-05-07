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
    #[value(name = "javascript", alias("js"))]
    JavaScript,
}

impl Display for Target {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        f.write_str(match self {
            Self::JavaScript => "javascript",
        })
    }
}

#[derive(Clone, Debug, Subcommand)]
pub enum Command {
    Format {
        /* options */
        #[arg(default_value = "**/*.kn")]
        glob: String,

        #[arg(short, long, default_value = "src")]
        root_dir: PathBuf,
    },

    Check {
        /* options */
        #[arg(short, long, default_value = ".")]
        root_dir: PathBuf,

        #[arg(short, long, default_value = "src")]
        source_dir: PathBuf,

        #[arg(short, long, default_value = "main.kn")]
        entry: PathBuf,
    },

    Build {
        target: Target,

        /* options */
        #[arg(short, long, default_value = "build")]
        out_dir: PathBuf,

        #[arg(short, long, default_value = ".")]
        root_dir: PathBuf,

        #[arg(short, long, default_value = "src")]
        source_dir: PathBuf,

        #[arg(short, long, default_value = "main.kn")]
        entry: PathBuf,
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
