use std::{
    fmt::Display,
    path::{Path, PathBuf},
};

#[cfg(feature = "format")]
pub mod format;
pub mod graph;

pub trait Generator: Copy {
    type Input;
    type Output: Display;

    fn generate(&self, path: &Path, input: Self::Input) -> (PathBuf, Self::Output);
}
