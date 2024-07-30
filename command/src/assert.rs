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

impl<T> AssertExists for T
where
    T: AsRef<Path>,
{
    fn assert_dir_exists<F>(self, factory: F) -> Result<Self>
    where
        F: Fn(PathBuf) -> ConfigurationError,
    {
        match fs::metadata(&self) {
            Ok(meta) if meta.is_dir() => Ok(self),
            _ => Err(Report::Configuration(factory(self.as_ref().to_path_buf()))),
        }
    }

    fn assert_file_exists<F>(self, factory: F) -> Result<Self>
    where
        F: Fn(PathBuf) -> ConfigurationError,
    {
        match fs::metadata(&self) {
            Ok(meta) if meta.is_file() => Ok(self),
            _ => Err(Report::Configuration(factory(self.as_ref().to_path_buf()))),
        }
    }
}
