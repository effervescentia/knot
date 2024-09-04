mod common;

use assert_cmd::Command;
use assert_fs::fixture::{FileWriteStr, PathChild, TempDir};
use std::{env, path::Path};

const SOURCE: &str = "const FOO = 123;";

fn command<P>(root_dir: P) -> Result<Command, Box<dyn std::error::Error>>
where
    P: AsRef<Path>,
{
    let mut cmd = Command::cargo_bin("knot")?;
    cmd.current_dir(root_dir);
    cmd.arg("check");
    cmd.arg("--target").arg("web");
    Ok(cmd)
}

#[test]
fn no_args() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir.child("src/main.kn").write_str(SOURCE)?;

    let mut cmd = command(&root_dir)?;

    cmd.assert().success();

    root_dir.close()?;

    Ok(())
}

#[test]
fn root_dir_arg() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir.child("src/main.kn").write_str(SOURCE)?;

    let mut cmd = command(env::current_dir()?)?;
    cmd.arg("--root-dir").arg(root_dir.path());

    cmd.assert().success();

    root_dir.close()?;

    Ok(())
}

#[test]
fn source_dir_arg() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir.child("source/main.kn").write_str(SOURCE)?;

    let mut cmd = command(&root_dir)?;
    cmd.arg("--source-dir").arg("source");

    cmd.assert().success();

    root_dir.close()?;

    Ok(())
}

#[test]
fn entry_arg() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir.child("src/entry.kn").write_str(SOURCE)?;

    let mut cmd = command(&root_dir)?;
    cmd.arg("--entry").arg("entry.kn");

    cmd.assert().success();

    root_dir.close()?;

    Ok(())
}
