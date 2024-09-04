mod common;

use assert_cmd::Command;
use assert_fs::{
    fixture::{PathChild, TempDir},
    prelude::FileWriteFile,
};
use std::path::{Path, PathBuf};

fn main_file(folder: &str) -> PathBuf {
    Path::new("../../examples/invalid")
        .join(folder)
        .join("src/main.kn")
}

fn command<P>(root_dir: P) -> Result<Command, Box<dyn std::error::Error>>
where
    P: AsRef<Path>,
{
    let mut cmd = Command::cargo_bin("knot")?;
    cmd.current_dir(&root_dir);
    cmd.arg("check");
    cmd.arg("--target").arg("web");
    Ok(cmd)
}

#[test]
fn not_found() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir
        .child("src/main.kn")
        .write_file(&main_file("401_not_found"))?;

    let mut cmd = command(&root_dir)?;

    cmd.assert().failure().stderr(predicates::str::contains(
        "Not Found (E#401)

  This expression references a variable named BAR which does not exist.",
    ));

    root_dir.close()?;

    Ok(())
}

#[test]
fn variant_not_found() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir
        .child("src/main.kn")
        .write_file(&main_file("402_variant_not_found"))?;

    let mut cmd = command(&root_dir)?;

    cmd.assert().failure().stderr(predicates::str::contains(
        "Variant Not Found (E#402)

  There is no variant named Bar declared on this enumerated type.",
    ));

    root_dir.close()?;

    Ok(())
}

#[test]
fn declaration_not_found() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir
        .child("src/main.kn")
        .write_file(&main_file("403_declaration_not_found"))?;

    let mut cmd = command(&root_dir)?;

    cmd.assert().failure().stderr(predicates::str::contains(
        "Declaration Not Found (E#403)

  There are no entities named bar declared in this module.",
    ));

    root_dir.close()?;

    Ok(())
}

#[test]
fn not_indexable() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir
        .child("src/main.kn")
        .write_file(&main_file("404_not_indexable"))?;

    let mut cmd = command(&root_dir)?;

    cmd.assert().failure().stderr(predicates::str::contains(
        "Not Indexable (E#404)

  The property foo cannot be accessed. This value does not support properties.",
    ));

    root_dir.close()?;

    Ok(())
}

#[ignore = "cannot trigger this error"]
#[test]
fn property_not_found() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir
        .child("src/main.kn")
        .write_file(&main_file("405_property_not_found"))?;

    let mut cmd = command(&root_dir)?;

    cmd.assert().failure().stderr(predicates::str::contains(
        "Property Not Found (E#405)

  There is no property named fizz on this value.",
    ));

    root_dir.close()?;

    Ok(())
}

#[test]
fn duplicate_property() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir
        .child("src/main.kn")
        .write_file(&main_file("406_duplicate_property"))?;

    let mut cmd = command(&root_dir)?;

    cmd.assert().failure().stderr(predicates::str::contains(
        "Duplicate Property (E#406)

  This expression includes multiple properties named bar.",
    ));

    root_dir.close()?;

    Ok(())
}

#[test]
fn not_spreadable() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir
        .child("src/main.kn")
        .write_file(&main_file("407_not_spreadable"))?;

    let mut cmd = command(&root_dir)?;

    cmd.assert().failure().stderr(predicates::str::contains(
        "Not Spreadable (E#407)

  This expression cannot be spread because it is not object-like.",
    ));

    root_dir.close()?;

    Ok(())
}

#[test]
fn untyped_parameter() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir
        .child("src/main.kn")
        .write_file(&main_file("408_untyped_parameter"))?;

    let mut cmd = command(&root_dir)?;

    cmd.assert().failure().stderr(predicates::str::contains(
        "Untyped Parameter (E#408)

  The parameter bar is missing a type annotation.",
    ));

    root_dir.close()?;

    Ok(())
}

#[test]
fn default_value_rejected() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir
        .child("src/main.kn")
        .write_file(&main_file("409_default_value_rejected"))?;

    let mut cmd = command(&root_dir)?;

    cmd.assert().failure().stderr(predicates::str::contains(
        "Default Value Rejected (E#409)

  The type of this default value does not match the parameter's type.",
    ));

    root_dir.close()?;

    Ok(())
}

#[test]
fn not_callable() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir
        .child("src/main.kn")
        .write_file(&main_file("410_not_callable"))?;

    let mut cmd = command(&root_dir)?;

    cmd.assert().failure().stderr(predicates::str::contains(
        "Not Callable (E#410)

  This expression is not a function and cannot be called.",
    ));

    root_dir.close()?;

    Ok(())
}

#[test]
fn unexpected_argument() -> Result<(), Box<dyn std::error::Error>> {
    let root_dir = TempDir::new()?;
    root_dir
        .child("src/main.kn")
        .write_file(&main_file("411_unexpected_argument"))?;

    let mut cmd = command(&root_dir)?;

    cmd.assert().failure().stderr(predicates::str::contains(
        "Unexpected Argument (E#411)

  This argument was not expected by the function call.",
    ));

    root_dir.close()?;

    Ok(())
}
