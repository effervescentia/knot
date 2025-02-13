mod common;

use assert_fs::prelude::*;
use assert_fs::TempDir;
use common::check_cmd;
use std::env;

const SOURCE: &str = "const FOO = 123;";

#[test]
fn no_args() {
    let root_dir = TempDir::new().unwrap();
    root_dir.child("src/main.kn").write_str(SOURCE).unwrap();

    let mut cmd = check_cmd(&root_dir).unwrap();

    cmd.assert().success();

    root_dir.close().unwrap();
}

#[test]
fn root_dir_arg() {
    let root_dir = TempDir::new().unwrap();
    root_dir.child("src/main.kn").write_str(SOURCE).unwrap();

    let mut cmd = check_cmd(env::current_dir().unwrap()).unwrap();
    cmd.arg("--root-dir").arg(root_dir.path());

    cmd.assert().success();

    root_dir.close().unwrap();
}

#[test]
fn source_dir_arg() {
    let root_dir = TempDir::new().unwrap();
    root_dir.child("source/main.kn").write_str(SOURCE).unwrap();

    let mut cmd = check_cmd(&root_dir).unwrap();
    cmd.arg("--source-dir").arg("source");

    cmd.assert().success();

    root_dir.close().unwrap();
}

#[test]
fn entry_arg() {
    let root_dir = TempDir::new().unwrap();
    root_dir.child("src/entry.kn").write_str(SOURCE).unwrap();

    let mut cmd = check_cmd(&root_dir).unwrap();
    cmd.arg("--entry").arg("entry.kn");

    cmd.assert().success();

    root_dir.close().unwrap();
}
