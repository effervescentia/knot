use std::{
    env, fs,
    path::{Path, PathBuf},
};

/// setup for all integration tests
pub fn setup() {}

pub fn test_path(relative: &str) -> PathBuf {
    let mut path = env::current_dir().unwrap();
    path.extend(vec![Path::new("tests"), Path::new(relative)]);

    path
}
