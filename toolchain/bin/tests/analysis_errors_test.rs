mod common;

use assert_fs::prelude::*;
use assert_fs::TempDir;
use common::check_cmd;
use std::path::{Path, PathBuf};

fn main_file(folder: &str) -> PathBuf {
    Path::new("../../examples/invalid")
        .join(folder)
        .join("src/main.kn")
}

#[test]
fn invoke_all_errors() -> Result<(), Box<dyn std::error::Error>> {
    let test_cases = vec![
        (
            "401_not_found",
            predicates::str::contains(
                "Not Found (E#401)

  This expression references a variable named BAR which does not exist.",
            ),
        ),
        (
            "402_variant_not_found",
            predicates::str::contains(
                "Variant Not Found (E#402)

  There is no variant named Bar declared on this enumerated type.",
            ),
        ),
        (
            "403_declaration_not_found",
            predicates::str::contains(
                "Declaration Not Found (E#403)

  There are no entities named bar declared in this module.",
            ),
        ),
        (
            "404_not_indexable",
            predicates::str::contains(
                "Not Indexable (E#404)

  The property foo cannot be accessed. This value does not support properties.",
            ),
        ),
        (
            "405_property_not_found",
            predicates::str::contains(
                "Property Not Found (E#405)

  There is no property named fizz on this value.",
            ),
        ),
        (
            "406_duplicate_property",
            predicates::str::contains(
                "Duplicate Property (E#406)

  This expression includes multiple properties named bar.",
            ),
        ),
        (
            "407_not_spreadable",
            predicates::str::contains(
                "Not Spreadable (E#407)

  This expression cannot be spread because it is not object-like.",
            ),
        ),
        (
            "408_untyped_parameter",
            predicates::str::contains(
                "Untyped Parameter (E#408)

  The parameter bar is missing a type annotation.",
            ),
        ),
        (
            "409_default_value_rejected",
            predicates::str::contains(
                "Default Value Rejected (E#409)

  The type of this default value does not match the parameter's type.",
            ),
        ),
        (
            "410_not_callable",
            predicates::str::contains(
                "Not Callable (E#410)

  This expression is not a function and cannot be called.",
            ),
        ),
        (
            "411_unexpected_argument",
            predicates::str::contains(
                "Unexpected Argument (E#411)

  This argument was not expected by the function call.",
            ),
        ),
    ];

    for (file, predicate) in test_cases {
        let root_dir = TempDir::new()?;
        root_dir.child("src/main.kn").write_file(&main_file(file))?;

        let mut cmd = check_cmd(&root_dir)?;

        cmd.assert().failure().stderr(predicate);

        root_dir.close()?;
    }

    Ok(())
}
