mod common;

use assert_cmd::Command;
use assert_fs::fixture::{FileWriteStr, PathChild, TempDir};

const SOURCE: &str = "const FOO = 123;";

#[test]
fn no_args() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir.child("src/main.kn").write_str(SOURCE)?;

    let mut cmd = Command::cargo_bin("knot")?;
    cmd.current_dir(&root_dir);
    cmd.arg("check");

    cmd.assert().success();

    root_dir.close()?;

    Ok(())
}

#[test]
fn root_dir_arg() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir.child("src/main.kn").write_str(SOURCE)?;

    let mut cmd = Command::cargo_bin("knot")?;
    cmd.arg("check");
    cmd.arg("--root-dir").arg(root_dir.path());

    cmd.assert().success();

    root_dir.close()?;

    Ok(())
}

#[test]
fn source_dir_arg() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir.child("source/main.kn").write_str(SOURCE)?;

    let mut cmd = Command::cargo_bin("knot")?;
    cmd.current_dir(&root_dir);
    cmd.arg("check");
    cmd.arg("--source-dir").arg("source");

    cmd.assert().success();

    root_dir.close()?;

    Ok(())
}

#[test]
fn entry_arg() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir.child("src/entry.kn").write_str(SOURCE)?;

    let mut cmd = Command::cargo_bin("knot")?;
    cmd.current_dir(&root_dir);
    cmd.arg("check");
    cmd.arg("--entry").arg("entry.kn");

    cmd.assert().success();

    root_dir.close()?;

    Ok(())
}
