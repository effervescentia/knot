use std::{env, fs, path::PathBuf};

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
    env::current_dir().unwrap().join("tests")
}

pub fn scratch_path() -> PathBuf {
    test_path().join(".scratch")
}
