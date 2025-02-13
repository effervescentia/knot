use super::Scope;
use std::{
    collections::HashSet,
    hash::Hash,
    path::{Path, PathBuf},
};

pub struct Input<Src, Lib> {
    pub source: Src,
    pub libraries: HashSet<Lib>,
}

impl<Src, Lib> Input<Src, Lib>
where
    Lib: Clone + Eq + Hash,
{
    fn from_source<T>(source: Src, libraries: T) -> Self
    where
        T: AsRef<[Lib]>,
    {
        Self {
            source,
            libraries: HashSet::from_iter(libraries.as_ref().to_vec()),
        }
    }
}

impl<Lib> Input<Entrypoint, Lib>
where
    Lib: Clone + Eq + Hash,
{
    pub fn from_entry<T, U>(entry: T, libraries: U) -> Self
    where
        T: AsRef<Path>,
        U: AsRef<[Lib]>,
    {
        Self::from_source(Entrypoint(entry.as_ref().to_path_buf()), libraries)
    }
}

impl<Lib> Input<Glob, Lib>
where
    Lib: Clone + Eq + Hash,
{
    pub fn from_glob<T, U>(glob: T, libraries: U) -> Self
    where
        T: AsRef<str>,
        U: AsRef<[Lib]>,
    {
        Self::from_source(Glob(glob.as_ref().to_owned()), libraries)
    }
}

pub trait Source {
    fn resolve(&self, source_dir: &Path) -> Scope;
}

pub struct Entrypoint(PathBuf);

impl Source for Entrypoint {
    // TODO: should return a result to handle error cases
    fn resolve(&self, source_dir: &Path) -> Scope {
        let absolute = source_dir.join(&self.0);

        let relative = absolute.strip_prefix(source_dir).unwrap().to_path_buf();

        vec![relative]
    }
}

pub struct Glob(String);

impl Source for Glob {
    // TODO: should handle error cases
    fn resolve(&self, source_dir: &Path) -> Scope {
        match glob::glob(&source_dir.join(&self.0).to_string_lossy()) {
            Ok(x) => x
                .flat_map(|x| match x {
                    Ok(absolute) => {
                        let relative = absolute.strip_prefix(source_dir).unwrap().to_path_buf();

                        vec![relative]
                    }
                    Err(_) => vec![],
                })
                .collect(),

            Err(_) => vec![],
        }
    }
}

#[cfg(test)]
mod tests {
    use super::{Input, Source};
    use assert_fs::{
        prelude::{FileTouch, PathChild},
        TempDir,
    };
    use kore::assert_eq;
    use std::{collections::HashSet, path::PathBuf};

    #[derive(Clone, Copy, PartialEq, Eq, Hash)]
    pub struct MockLibrary;

    #[test]
    fn input_from_glob() -> Result<(), Box<dyn std::error::Error>> {
        let root_dir = TempDir::new()?;

        root_dir.child("main.kn").touch()?;
        root_dir.child("foo/foo.kn").touch()?;
        root_dir.child("bar/bar.kn").touch()?;

        let resolved = Input::from_glob("**/*.kn", [MockLibrary])
            .source
            .resolve(&root_dir);

        assert_eq!(
            HashSet::from_iter(resolved),
            HashSet::from([
                PathBuf::from("main.kn"),
                PathBuf::from("foo/foo.kn"),
                PathBuf::from("bar/bar.kn")
            ])
        );

        Ok(())
    }

    #[test]
    fn input_from_entry() -> Result<(), Box<dyn std::error::Error>> {
        let root_dir = TempDir::new()?;

        root_dir.child("foo/bar/main.kn").touch()?;

        let resolved = Input::from_entry("foo/bar/main.kn", [MockLibrary])
            .source
            .resolve(&root_dir);

        assert_eq!(resolved, vec![PathBuf::from("foo/bar/main.kn")]);

        Ok(())
    }
}
