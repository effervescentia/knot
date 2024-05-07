use std::path::{Path, PathBuf, StripPrefixError};

use assert_cmd::Command;
use assert_fs::fixture::{FileWriteStr, PathChild, PathCreateDir, TempDir};

fn prefix_private(path: &Path) -> Result<PathBuf, StripPrefixError> {
    Ok(Path::new("/private").join(path.strip_prefix("/")?))
}

#[test]
fn root_directory_not_found() -> Result<(), Box<dyn std::error::Error>> {
    let temp_dir = TempDir::new()?;
    let root_dir = temp_dir.path().join("does_not_exist");

    let mut cmd = Command::cargo_bin("knot")?;
    cmd.arg("build").arg("javascript");
    cmd.arg("--root-dir").arg(&root_dir);

    cmd.assert()
        .failure()
        .stderr(predicates::str::contains(format!(
            "Root Directory Not Found (E#111)

no folder was found at the path {}",
            root_dir.display()
        )));

    temp_dir.close()?;

    Ok(())
}

#[test]
fn source_directory_not_found() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    let source_dir = root_dir.child("src");

    let mut cmd = Command::cargo_bin("knot")?;
    cmd.current_dir(&root_dir);
    cmd.arg("build").arg("javascript");

    cmd.assert()
        .failure()
        .stderr(predicates::str::contains(format!(
            "Source Directory Not Found (E#112)

no folder was found at the path {}",
            prefix_private(&source_dir)?.display()
        )));

    root_dir.close()?;

    Ok(())
}

#[test]
fn source_directory_not_relative() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    let source_dir = root_dir.child("src");

    let mut cmd = Command::cargo_bin("knot")?;
    cmd.current_dir(&root_dir);
    cmd.arg("build").arg("javascript");
    cmd.arg("--source-dir").arg(source_dir.path());

    cmd.assert()
        .failure()
        .stderr(predicates::str::contains(format!(
            "Source Directory Not Relative (E#113)

the path to the source directory should be relative to the root_dir but found {}",
            source_dir.display()
        )));

    root_dir.close()?;

    Ok(())
}

#[test]
fn entrypoint_not_found() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    let source_dir = root_dir.child("src");
    let entry = source_dir.child("main.kn");
    source_dir.create_dir_all()?;

    let mut cmd = Command::cargo_bin("knot")?;
    cmd.current_dir(&root_dir);
    cmd.arg("build").arg("javascript");

    cmd.assert()
        .failure()
        .stderr(predicates::str::contains(format!(
            "Entrypoint Not Found (E#114)

no module was found at the path {}",
            prefix_private(&entry)?.display()
        )));

    root_dir.close()?;

    Ok(())
}

#[test]
fn entrypoint_not_relative() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    let entry = root_dir.child("src/main.kn");
    entry.write_str("const FOO = 123")?;

    let mut cmd = Command::cargo_bin("knot")?;
    cmd.current_dir(&root_dir);
    cmd.arg("build").arg("javascript");
    cmd.arg("--entry").arg(entry.path());

    cmd.assert()
        .failure()
        .stderr(predicates::str::contains(format!(
            "Entrypoint Not Relative (E#115)

the path to the entrypoint should be relative to the source_dir but found {}",
            entry.display()
        )));

    root_dir.close()?;

    Ok(())
}
