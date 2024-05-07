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
