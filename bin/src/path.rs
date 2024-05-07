use engine::{Error, Result};
use std::{
    fs,
    path::{Path, PathBuf},
};

pub fn get_root_dir(path: &Path) -> Result<PathBuf> {
    if path.is_absolute() {
        path.to_path_buf()
    } else {
        path.canonicalize()
            .map_err(|_| vec![Error::RootDirectoryNotFound(path.to_path_buf())])?
    }
    .assert_dir_exists(Error::RootDirectoryNotFound)
}

pub fn get_source_dir(root_dir: &Path, path: &Path) -> Result<PathBuf> {
    if path.is_absolute() {
        return Err(vec![Error::SourceDirectoryNotRelative(path.to_path_buf())]);
    }

    root_dir
        .join(path)
        .assert_dir_exists(Error::SourceDirectoryNotFound)
}

pub fn get_out_dir(root_dir: &Path, path: &Path) -> PathBuf {
    if path.is_absolute() {
        path.to_path_buf()
    } else {
        root_dir.join(path)
    }
}

pub fn validate_entrypoint(source_dir: &Path, path: &Path) -> Result<()> {
    if path.is_absolute() {
        return Err(vec![Error::EntrypointNotRelative(path.to_path_buf())]);
    }

    source_dir
        .join(path)
        .assert_file_exists(Error::EntrypointNotFound)?;

    Ok(())
}

pub trait AssertExists: Sized {
    fn assert_dir_exists<F>(self, factory: F) -> Result<Self>
    where
        F: Fn(Self) -> Error;

    fn assert_file_exists<F>(self, factory: F) -> Result<Self>
    where
        F: Fn(Self) -> Error;
}

impl<T> AssertExists for T
where
    T: AsRef<Path>,
{
    fn assert_dir_exists<F>(self, factory: F) -> Result<Self>
    where
        F: Fn(Self) -> Error,
    {
        match fs::metadata(&self) {
            Ok(meta) if meta.is_dir() => Ok(self),
            _ => Err(vec![factory(self)]),
        }
    }

    fn assert_file_exists<F>(self, factory: F) -> Result<Self>
    where
        F: Fn(Self) -> Error,
    {
        match fs::metadata(&self) {
            Ok(meta) if meta.is_file() => Ok(self),
            _ => Err(vec![factory(self)]),
        }
    }
}
