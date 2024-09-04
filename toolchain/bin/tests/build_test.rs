mod common;

use assert_cmd::Command;
use assert_fs::{
    assert::PathAssert,
    fixture::{FileWriteStr, PathChild, TempDir},
};
use common::AssertDirContents;
use std::{env, path::Path};

const SOURCE: &str = "const FOO = 123;";
const JAVASCRIPT: &str = "import { $knot } from \"@knot/runtime\";
var FOO = 123;
export { FOO };
";

fn command<P>(root_dir: P) -> Result<Command, Box<dyn std::error::Error>>
where
    P: AsRef<Path>,
{
    let mut cmd = Command::cargo_bin("knot")?;
    cmd.current_dir(root_dir);
    cmd.arg("build");
    cmd.arg("--target").arg("web");
    Ok(cmd)
}

#[test]
fn web_target() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir.child("src/main.kn").write_str(SOURCE)?;

    let mut cmd = command(&root_dir)?;

    cmd.assert().success();
    root_dir.child("build").assert_dir_contents(&["main.js"]);

    root_dir.child("build/main.js").assert(JAVASCRIPT);

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
    root_dir.child("build").assert_dir_contents(&["main.js"]);

    root_dir.child("build/main.js").assert(JAVASCRIPT);

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
    root_dir.child("build").assert_dir_contents(&["main.js"]);

    root_dir.child("build/main.js").assert(JAVASCRIPT);

    root_dir.close()?;

    Ok(())
}

#[test]
fn out_dir_arg() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    let out_dir = TempDir::new()?;
    root_dir.child("src/main.kn").write_str(SOURCE)?;

    let mut cmd = command(&root_dir)?;
    cmd.arg("--out-dir").arg(out_dir.path());

    cmd.assert().success();
    out_dir.assert_dir_contents(&["main.js"]);

    out_dir.child("main.js").assert(JAVASCRIPT);

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
    root_dir.child("build").assert_dir_contents(&["entry.js"]);

    root_dir.child("build/entry.js").assert(JAVASCRIPT);

    root_dir.close()?;

    Ok(())
}
