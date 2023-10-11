use std::{
    env,
    path::{Path, PathBuf},
};

pub fn test_path(relative: &str) -> PathBuf {
    let mut path = env::current_dir().unwrap();
    path.extend(vec![Path::new("tests"), Path::new(relative)]);

    path
}
