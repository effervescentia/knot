use super::scratch_path;
use knot_command::{
    engine,
    format::{self, Options},
};
use std::fs;

pub fn format(test_name: &str, input: &str) -> engine::Result<String> {
    let source_dir = scratch_path().join(test_name);
    let entry = source_dir.join(test_name).with_extension("kn");

    fs::create_dir(&source_dir).expect(&format!(
        "failed to create output directory {}",
        source_dir.display()
    ));
    fs::write(&entry, input).expect("failed to write input file to disk");

    let result = format::command(&Options {
        source_dir: source_dir.as_path(),
        glob: "*.kn",
    });

    result.map(|_| fs::read_to_string(entry).expect("failed to read output file from disk"))
}
