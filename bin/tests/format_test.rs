mod common;

use assert_cmd::Command;
use assert_fs::{
    assert::PathAssert,
    fixture::{FileWriteStr, PathChild, TempDir},
};

const INPUT: &str = "  const   \nFOO  = \n 123  \n;  ";
const OUTPUT: &str = "const FOO = 123;\n";

#[test]
fn no_args() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    let entry = root_dir.child("src/main.kn");
    entry.write_str(INPUT)?;

    let mut cmd = Command::cargo_bin("knot")?;
    cmd.current_dir(&root_dir);
    cmd.arg("format");

    cmd.assert().success();

    entry.assert(OUTPUT);

    root_dir.close()?;

    Ok(())
}

#[test]
fn root_dir_arg() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    let entry = root_dir.child("src/main.kn");
    entry.write_str(INPUT)?;

    let mut cmd = Command::cargo_bin("knot")?;
    cmd.arg("format");
    cmd.arg("--root-dir").arg(root_dir.path());

    cmd.assert().success();

    entry.assert(OUTPUT);

    root_dir.close()?;

    Ok(())
}

#[test]
fn glob_arg() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    let file_a = root_dir.child("src/file_a.kn");
    let file_b = root_dir.child("src/file_b.kn");
    let file_c = root_dir.child("src/file_c.kn");
    file_a.write_str(INPUT)?;
    file_b.write_str(INPUT)?;
    file_c.write_str(INPUT)?;

    let mut cmd = Command::cargo_bin("knot")?;
    cmd.current_dir(&root_dir);
    cmd.arg("format");
    cmd.arg("**/*_a.kn");

    cmd.assert().success();

    file_a.assert(OUTPUT);
    file_b.assert(INPUT);
    file_c.assert(INPUT);

    root_dir.close()?;

    Ok(())
}

#[test]
fn multiple() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    let file_a = root_dir.child("src/file_a.kn");
    let file_b = root_dir.child("src/file_b.kn");
    let file_c = root_dir.child("src/file_c.kn");
    file_a.write_str(INPUT)?;
    file_b.write_str(INPUT)?;
    file_c.write_str(INPUT)?;

    let mut cmd = Command::cargo_bin("knot")?;
    cmd.current_dir(&root_dir);
    cmd.arg("format");

    cmd.assert().success();

    file_a.assert(OUTPUT);
    file_b.assert(OUTPUT);
    file_c.assert(OUTPUT);

    root_dir.close()?;

    Ok(())
}

#[test]
fn ignore_semantics() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    let entry = root_dir.child("src/main.kn");
    entry.write_str("  const \n  FOO   =  BAR\n ;  ")?;

    let mut cmd = Command::cargo_bin("knot")?;
    cmd.current_dir(&root_dir);
    cmd.arg("format");

    cmd.assert().success();

    entry.assert("const FOO = BAR;\n");

    root_dir.close()?;

    Ok(())
}
