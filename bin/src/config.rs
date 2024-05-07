use kore::{color::Highlight, pretty::Pretty};
use std::{fmt::Display, path::Path};

use crate::args::Target;

pub enum Config<'a> {
    Path(&'a Path),

    RelativePath {
        relative: &'a Path,
        absolute: &'a Path,
    },

    Target(&'a Target),

    String(&'a str),
}

impl<'a> Config<'a> {
    pub const fn rel_path(relative: &'a Path, absolute: &'a Path) -> Self {
        Self::RelativePath { relative, absolute }
    }
}

impl<'a> Display for Config<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            Self::Path(path) => path.pretty().fmt(f),

            Self::RelativePath { relative, absolute } => write!(
                f,
                "{} {}",
                relative.pretty(),
                format!("({})", absolute.display()).subtle()
            ),

            Self::Target(x) => x.to_string().highlight().fmt(f),

            Self::String(x) => x.highlight().fmt(f),
        }
    }
}

pub struct ConfigList<'a>(Vec<(&'static str, Config<'a>)>);

impl<'a> ConfigList<'a> {
    pub fn new(configs: Vec<(&'static str, Config<'a>)>) -> Self {
        Self(configs)
    }
}

impl<'a> Display for ConfigList<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        let max_len = self
            .0
            .iter()
            .map(|(key, _)| key.len())
            .max()
            .unwrap_or_default();
        let sep_offset = max_len + 1;

        for (key, value) in &self.0 {
            write!(f, "{:width$}: ", key, width = sep_offset)?;
            value.fmt(f)?;
            writeln!(f)?;
        }

        Ok(())
    }
}
