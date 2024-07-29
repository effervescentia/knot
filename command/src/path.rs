use engine::{ConfigurationError, Report, Result};
use std::{
    fs,
    path::{Path, PathBuf},
};

pub trait AssertExists: Sized {
    fn assert_dir_exists<F>(self, factory: F) -> Result<Self>
    where
        F: Fn(PathBuf) -> ConfigurationError;

    fn assert_file_exists<F>(self, factory: F) -> Result<Self>
    where
        F: Fn(PathBuf) -> ConfigurationError;
}

impl<'a> AssertExists for &'a Path {
    fn assert_dir_exists<F>(self, factory: F) -> Result<Self>
    where
        F: Fn(PathBuf) -> ConfigurationError,
    {
        match fs::metadata(self) {
            Ok(meta) if meta.is_dir() => Ok(self),
            _ => Err(Report::Configuration(factory(self.to_path_buf()))),
        }
    }

    fn assert_file_exists<F>(self, factory: F) -> Result<Self>
    where
        F: Fn(PathBuf) -> ConfigurationError,
    {
        match fs::metadata(self) {
            Ok(meta) if meta.is_file() => Ok(self),
            _ => Err(Report::Configuration(factory(self.to_path_buf()))),
        }
    }
}
