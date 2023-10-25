use super::scratch_path;
use knot_command::build;
use kore::Generator;
use lang::ast::ProgramShape;
use std::{collections::HashMap, fs, path::Path};

fn write_files(source_dir: &Path, files: &Vec<(&str, &str)>) {
    files.iter().for_each(|(name, data)| {
        let path = source_dir.join(name);

        if let Some(parent) = path.parent() {
            fs::create_dir_all(parent).ok();
        }

        fs::write(&path, data).expect(&format!(
            "failed to write file with path {}",
            path.display()
        ))
    });
}

fn collect_files(out_dir: &Path) -> HashMap<String, String> {
    let iter = glob::glob(&format!(
        "{}/**/*",
        out_dir.to_str().expect("failed to convert path to string")
    ))
    .expect("failed to compile glob")
    .filter_map(|x| {
        let path = x.expect("failed to apply glob");
        if path.is_dir() {
            None
        } else {
            Some(path)
        }
    })
    .map(|x| {
        (
            x.strip_prefix(&out_dir)
                .expect("failed to make path relative")
                .to_str()
                .expect("failed to convert path to string")
                .to_string(),
            fs::read_to_string(&x).expect(&format!("failed to read file {}", x.display())),
        )
    });

    HashMap::from_iter(iter)
}

pub fn build<G>(test_name: &str, files: Vec<(&str, &str)>, generator: G) -> HashMap<String, String>
where
    G: Generator<Input = ProgramShape>,
{
    let source_dir = scratch_path().join(format!("{test_name}_source"));
    let out_dir = scratch_path().join(format!("{test_name}_output"));

    fs::create_dir(&source_dir).expect("failed to create source directory");
    fs::create_dir(&out_dir).expect("failed to create output directory");

    write_files(&source_dir, &files);

    let (entry, _) = files.first().expect("files list was empty");

    build::command(&build::Options {
        generator,
        entry: Path::new(entry),
        source_dir: source_dir.as_path(),
        out_dir: out_dir.as_path(),
    });

    collect_files(&out_dir)
}
