#![allow(dead_code, unused_imports, clippy::expect_used, clippy::create_dir)]
mod build;
mod format;

pub use build::build;
use engine::Library;
pub use format::format;
use kore::{assert_eq_sorted, str};
use std::{
    collections::HashMap,
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

pub fn assert_build_single(name: &str, input: &str, output: &str) {
    let name = test_name(file!(), &name.replace("::", "-"));
    let result = build(
        &name,
        &[("main.kn", input)],
        js::JavaScriptGenerator::<Library>::new(js::Module::ESM),
    );

    assert_eq_sorted!(
        result.unwrap(),
        HashMap::from_iter(vec![(str!("main.js"), output.to_owned())])
    );
}
