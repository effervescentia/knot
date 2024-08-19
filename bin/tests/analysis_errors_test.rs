mod common;

use assert_cmd::Command;
use assert_fs::fixture::{FileWriteStr, PathChild, TempDir};

#[test]
fn not_found() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir
        .child("src/main.kn")
        .write_str("const FOO = BAR;")?;

    let mut cmd = Command::cargo_bin("knot")?;
    cmd.current_dir(&root_dir);
    cmd.arg("check");

    cmd.assert().failure().stderr(predicates::str::contains(
        "Not Found (E#401)

  This expression references a variable named BAR which does not exist.",
    ));

    root_dir.close()?;

    Ok(())
}
