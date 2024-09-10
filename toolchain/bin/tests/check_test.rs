mod common;

use assert_fs::prelude::*;
use assert_fs::TempDir;
use common::check_cmd;
use std::env;

const SOURCE: &str = "const FOO = 123;";

#[test]
fn no_args() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir.child("src/main.kn").write_str(SOURCE)?;

    let mut cmd = check_cmd(&root_dir)?;

    cmd.assert().success();

    root_dir.close()?;

    Ok(())
}

#[test]
fn root_dir_arg() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir.child("src/main.kn").write_str(SOURCE)?;

    let mut cmd = check_cmd(env::current_dir()?)?;
    cmd.arg("--root-dir").arg(root_dir.path());

    cmd.assert().success();

    root_dir.close()?;

    Ok(())
}

#[test]
fn source_dir_arg() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir.child("source/main.kn").write_str(SOURCE)?;

    let mut cmd = check_cmd(&root_dir)?;
    cmd.arg("--source-dir").arg("source");

    cmd.assert().success();

    root_dir.close()?;

    Ok(())
}

#[test]
fn entry_arg() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir.child("src/entry.kn").write_str(SOURCE)?;

    let mut cmd = check_cmd(&root_dir)?;
    cmd.arg("--entry").arg("entry.kn");

    cmd.assert().success();

    root_dir.close()?;

    Ok(())
}
