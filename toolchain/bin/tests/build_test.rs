mod common;

use assert_fs::prelude::*;
use assert_fs::TempDir;
use common::{build_cmd, AssertDirContents};
use std::env;

const SOURCE: &str = "const FOO = 123;";
const JAVASCRIPT: &str = "import { $knot } from \"@knot/runtime\";
var FOO = 123;
export { FOO };
";

#[test]
fn web_target() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir.child("src/main.kn").write_str(SOURCE)?;

    let mut cmd = build_cmd(&root_dir)?;

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

    let mut cmd = build_cmd(env::current_dir()?)?;
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

    let mut cmd = build_cmd(&root_dir)?;
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

    let mut cmd = build_cmd(&root_dir)?;
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

    let mut cmd = build_cmd(&root_dir)?;
    cmd.arg("--entry").arg("entry.kn");

    cmd.assert().success();
    root_dir.child("build").assert_dir_contents(&["entry.js"]);

    root_dir.child("build/entry.js").assert(JAVASCRIPT);

    root_dir.close()?;

    Ok(())
}
