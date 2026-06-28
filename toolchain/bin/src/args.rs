use clap::{Parser, Subcommand, ValueEnum};
use kore::internal;
use std::{fmt::Display, path::PathBuf};

#[derive(Debug, Parser)]
#[command(name = "knot")]
#[command(version = "3.0")]
#[command(about, long_about = None)]
pub struct Args {
    #[command(subcommand)]
    pub command: Command,

    #[arg(short, long)]
    pub verbose: bool,
}

#[derive(Copy, Clone, Debug, Eq, Ord, PartialEq, PartialOrd, ValueEnum)]
pub enum Target {
    #[value(name = "web")]
    Web,
}

impl Target {
    pub fn to_platform(self) -> impl internal::Platform<Program = lang::ast::shape::Program> {
        match self {
            Self::Web => web::Web,
        }
    }
}

impl Display for Target {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        f.write_str(match self {
            Self::Web => "web",
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
        #[arg(short, long)]
        target: Target,

        #[arg(short, long, default_value = ".")]
        root_dir: PathBuf,

        #[arg(short, long, default_value = "src")]
        source_dir: PathBuf,

        #[arg(short, long, default_value = "main.kn")]
        entry: PathBuf,
    },

    Build {
        /* options */
        #[arg(short, long)]
        target: Target,

        #[arg(short, long, default_value = "build")]
        out_dir: PathBuf,

        #[arg(short, long, default_value = ".")]
        root_dir: PathBuf,

        #[arg(short, long, default_value = "src")]
        source_dir: PathBuf,

        #[arg(short, long, default_value = "main.kn")]
        entry: PathBuf,

        #[arg(short, long, default_value = "false")]
        watch: bool,
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
