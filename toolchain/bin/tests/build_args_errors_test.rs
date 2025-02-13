mod common;

use assert_fs::prelude::*;
use assert_fs::TempDir;
use common::build_cmd;
use std::{
    env,
    path::{Path, PathBuf, StripPrefixError},
};

fn prefix_private(path: &Path) -> Result<PathBuf, StripPrefixError> {
    if env::consts::ARCH == "aarch64" {
        return Ok(Path::new("/private").join(path.strip_prefix("/").unwrap()));
    }

    Ok(path.to_path_buf())
}

#[test]
fn root_directory_not_found() {
    let temp_dir = TempDir::new().unwrap();
    let root_dir = temp_dir.path().join("does_not_exist");

    let mut cmd = build_cmd(env::current_dir().unwrap()).unwrap();
    cmd.arg("--root-dir").arg(&root_dir);

    cmd.assert()
        .failure()
        .stderr(predicates::str::contains(format!(
            "Root Directory Not Found (E#111)

  No folder was found at the path {}.",
            root_dir.display()
        )));

    temp_dir.close().unwrap();
}

#[test]
fn source_directory_not_found() {
    let root_dir = TempDir::new().unwrap();
    let source_dir = root_dir.child("src");

    let mut cmd = build_cmd(&root_dir).unwrap();

    cmd.assert()
        .failure()
        .stderr(predicates::str::contains(format!(
            "Source Directory Not Found (E#112)

  No folder was found at the path {}.",
            prefix_private(&source_dir).unwrap().display()
        )));

    root_dir.close().unwrap();
}

#[test]
fn source_directory_not_relative() {
    let root_dir = TempDir::new().unwrap();
    let source_dir = root_dir.child("src");

    let mut cmd = build_cmd(&root_dir).unwrap();
    cmd.arg("--source-dir").arg(source_dir.path());

    cmd.assert()
        .failure()
        .stderr(predicates::str::contains(format!(
            "Source Directory Not Relative (E#113)

  The path to the source directory should be relative to the root_dir but found {}.",
            source_dir.display()
        )));

    root_dir.close().unwrap();
}

#[test]
fn entrypoint_not_found() {
    let root_dir = TempDir::new().unwrap();
    let source_dir = root_dir.child("src");
    let entry = source_dir.child("main.kn");
    source_dir.create_dir_all().unwrap();

    let mut cmd = build_cmd(&root_dir).unwrap();

    cmd.assert()
        .failure()
        .stderr(predicates::str::contains(format!(
            "Entrypoint Not Found (E#114)

  No module was found at the path {}.",
            prefix_private(&entry).unwrap().display()
        )));

    root_dir.close().unwrap();
}

#[test]
fn entrypoint_not_relative() {
    let root_dir = TempDir::new().unwrap();
    let entry = root_dir.child("src/main.kn");
    entry.write_str("const FOO = 123").unwrap();

    let mut cmd = build_cmd(&root_dir).unwrap();
    cmd.arg("--entry").arg(entry.path());

    cmd.assert()
        .failure()
        .stderr(predicates::str::contains(format!(
            "Entrypoint Not Relative (E#115)

  The path to the entrypoint should be relative to the source_dir but found {}.",
            entry.display()
        )));

    root_dir.close().unwrap();
}
