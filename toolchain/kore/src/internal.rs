use std::{
    collections::HashSet,
    fmt::Display,
    path::{Path, PathBuf},
    str::FromStr,
};

#[derive(PartialEq)]
pub enum Mode {
    Development,
    Production,
}

pub trait PlatformLibrary: Copy + Into<Library> + From<Library> {
    fn text(&self) -> &str;

    fn module(&self) -> Option<&'static str>;
}

pub trait Generator: Copy {
    type Input;
    type Output: Display;

    fn generate(&self, path: &Path, input: Self::Input) -> (PathBuf, Self::Output);
}

pub trait Platform {
    type Library: PlatformLibrary;
    type Generator: Generator<Input = Self::Program>;
    type Program;

    fn libraries() -> HashSet<Self::Library>;

    fn generator() -> Self::Generator;
}

#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq)]
pub enum AmbientScope {
    /// defines the elements available for the active platform, eg. HTML tags in the browser
    Element,

    /// defines the styling rules available for the active platform, eg. CSS in the browser
    Style,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum Library {
    /// provides native bindings for the standard library
    Std,

    /// provides native bindings for ambient scope features
    Ambient(AmbientScope),
}

impl Library {
    pub const fn to_ambient_scope(&self) -> Option<AmbientScope> {
        match self {
            Self::Ambient(ambient) => Some(*ambient),

            Self::Std => None,
        }
    }
}

impl FromStr for Library {
    type Err = ();

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "std" => Ok(Self::Std),

            _ => Err(()),
        }
    }
}
