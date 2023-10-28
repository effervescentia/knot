#[cfg(feature = "format")]
pub mod format;

#[cfg(feature = "test")]
pub use pretty_assertions::{assert_eq, assert_ne, assert_str_eq};
use std::{
    fmt::Display,
    path::{Path, PathBuf},
};

pub trait Generator: Copy {
    type Input;
    type Output: Display;

    fn generate(&self, path: &Path, input: Self::Input) -> (PathBuf, Self::Output);
}
