use command::AssertExists;
use engine::{ConfigurationError, Report, Result};
use std::path::{Path, PathBuf};

pub fn get_root_dir(path: &Path) -> Result<PathBuf> {
    if path.is_absolute() {
        path.to_path_buf()
    } else {
        path.canonicalize().map_err(|_| {
            Report::Configuration(ConfigurationError::RootDirectoryNotFound(
                path.to_path_buf(),
            ))
        })?
    }
    .assert_dir_exists(ConfigurationError::RootDirectoryNotFound)
}

pub fn get_source_dir(root_dir: &Path, path: &Path) -> Result<PathBuf> {
    if path.is_absolute() {
        return Err(Box::new(Report::Configuration(
            ConfigurationError::SourceDirectoryNotRelative(path.to_path_buf()),
        )));
    }

    root_dir
        .join(path)
        .assert_dir_exists(ConfigurationError::SourceDirectoryNotFound)
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
        return Err(Box::new(Report::Configuration(
            ConfigurationError::EntrypointNotRelative(path.to_path_buf()),
        )));
    }

    source_dir
        .join(path)
        .assert_file_exists(ConfigurationError::EntrypointNotFound)?;

    Ok(())
}
