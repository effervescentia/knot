use super::Resolver;
use std::{
    fs,
    path::{Path, PathBuf},
    time::SystemTime,
};

pub struct FileSystem<'a>(pub &'a Path);

impl<'a> FileSystem<'a> {
    fn get_file_path<P>(&self, relative: P) -> PathBuf
    where
        P: AsRef<Path>,
    {
        self.0.join(relative)
    }
}

impl<'a> Resolver for FileSystem<'a> {
    fn resolve<P>(&mut self, relative: P) -> Option<String>
    where
        P: AsRef<Path>,
    {
        let path = self.get_file_path(relative);

        fs::read_to_string(path.as_path()).ok()
    }

    fn last_modified<P>(&self, relative: P) -> Option<SystemTime>
    where
        P: AsRef<Path>,
    {
        let path = self.get_file_path(relative);
        let metadata = fs::metadata(path).ok()?;

        metadata.modified().ok()
    }
}

#[cfg(test)]
mod tests {
    use super::FileSystem;
    use crate::resolve::Resolver;
    use std::{fs::File, io::Write, path::Path};
    use tempfile::tempdir;

    impl<'a> FileSystem<'a> {
        pub fn write(&self, path: &str, contents: &str) {
            let mut file = File::create(self.0.join(path)).unwrap();
            write!(file, "{}", contents).unwrap();
        }
    }

    const TARGET_FILE: &str = "target_file.txt";
    const FILE_CONTENTS: &str = "what's in the box?!";

    #[test]
    fn get_file_path() {
        let file_system = FileSystem(Path::new("foo/bar"));

        assert_eq!(
            file_system.get_file_path(Path::new(TARGET_FILE)),
            Path::new(&format!("foo/bar/{}", TARGET_FILE))
        );
    }

    #[test]
    fn resolve() {
        let dir = tempdir().unwrap();
        let mut file_system = FileSystem(dir.path());
        file_system.write(TARGET_FILE, FILE_CONTENTS);

        assert_eq!(
            file_system.resolve(Path::new(TARGET_FILE)),
            Some(FILE_CONTENTS.to_string())
        );
    }

    #[test]
    fn last_modified() {
        let dir = tempdir().unwrap();
        let file_system = FileSystem(dir.path());
        file_system.write(TARGET_FILE, FILE_CONTENTS);

        assert!(file_system.last_modified(Path::new(TARGET_FILE)).is_some());
    }
}
