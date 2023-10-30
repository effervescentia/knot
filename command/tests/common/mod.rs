#![allow(dead_code, clippy::expect_used, clippy::create_dir)]
mod build;
mod format;

pub use build::build;
pub use format::format;
use std::{
    env, fs,
    path::{Path, PathBuf},
};

/// setup for all integration tests
#[ctor::ctor]
fn setup() {
    let scratch_dir = scratch_path();

    if fs::metadata(scratch_dir.as_path()).is_ok() {
        fs::remove_dir_all(scratch_dir.as_path()).expect("failed to remove .scratch directory");
    }

    fs::create_dir(scratch_dir.as_path()).expect("failed to create .scratch directory");
}

pub fn test_path() -> PathBuf {
    env::current_dir()
        .expect("failed to get working directory")
        .join("tests")
}

pub fn scratch_path() -> PathBuf {
    test_path().join(".scratch")
}

pub fn test_name(file: &str, suffix: &str) -> String {
    let file_name = Path::new(file)
        .with_extension("")
        .file_name()
        .expect("failed to get test file name")
        .to_str()
        .expect("failed to convert file name to string")
        .to_owned();

    format!("{}_{suffix}", file_name)
}
