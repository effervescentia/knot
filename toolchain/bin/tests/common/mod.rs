#![allow(dead_code)]
use assert_cmd::Command;
use std::{collections::HashSet, path::Path};

pub fn check_cmd<P>(root_dir: P) -> Result<Command, Box<dyn std::error::Error>>
where
    P: AsRef<Path>,
{
    let mut cmd = Command::cargo_bin("knot")?;
    cmd.current_dir(root_dir);
    cmd.arg("check");
    cmd.arg("--target").arg("web");
    Ok(cmd)
}

pub fn build_cmd<P>(root_dir: P) -> Result<Command, Box<dyn std::error::Error>>
where
    P: AsRef<Path>,
{
    let mut cmd = Command::cargo_bin("knot")?;
    cmd.current_dir(root_dir);
    cmd.arg("build");
    cmd.arg("--target").arg("web");
    Ok(cmd)
}

pub trait AssertDirContents {
    fn assert_dir_contents(&self, expected_contents: &[&str]);
}

impl<P> AssertDirContents for P
where
    P: AsRef<Path>,
{
    #[allow(clippy::panic)]
    fn assert_dir_contents(&self, expected_contents: &[&str]) {
        let mut actual_contents = self.as_ref().read_dir().unwrap_or_else(|_| {
            panic!(
                "failed to read the contents of directory {}",
                self.as_ref().to_string_lossy()
            )
        });

        let actual_set = actual_contents
            .by_ref()
            .map(|file| {
                file.as_ref()
                    .unwrap_or_else(|_| panic!("failed to find a file at the path {:?}", file))
                    .path()
                    .strip_prefix(self)
                    .unwrap()
                    .to_string_lossy()
                    .to_string()
            })
            .collect::<HashSet<_>>();

        let expected_set = expected_contents
            .iter()
            .map(ToString::to_string)
            .collect::<HashSet<_>>();

        assert_eq!(
            expected_set,
            actual_set,
            "directory contents did not match expectations\n\nexpected: {:?}\n\nactual {:?}",
            expected_contents,
            actual_contents
                .map(|x| x.map(|x| x.file_name()).unwrap_or_default())
                .collect::<Vec<_>>(),
        );
    }
}
