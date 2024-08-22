use analyze::AmbientScope;
use kore::Serializable;
use std::{fmt::Display, str::FromStr};

#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq)]
pub enum Library {
    Std,
    Html,
}

impl Library {
    pub const fn resolve(&self) -> &str {
        match self {
            Self::Std => library::STD,
            Self::Html => library::HTML,
        }
    }

    pub const fn to_ambient_scope(&self) -> Option<AmbientScope> {
        match self {
            Self::Html => Some(AmbientScope::Element),

            Self::Std => None,
        }
    }
}

impl Serializable for Library {}

impl FromStr for Library {
    type Err = ();

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "std" => Ok(Self::Std),

            _ => Err(()),
        }
    }
}

impl Display for Library {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        f.write_str(match self {
            Self::Std => "std",
            Self::Html => "html",
        })
    }
}

mod library {
    pub const STD: &str = include_str!("lib_std.kd");
    pub const HTML: &str = include_str!("lib_html.kd");
}
