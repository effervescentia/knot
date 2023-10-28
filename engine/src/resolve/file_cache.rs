use super::{file_system::FileSystem, Resolver};
use std::{fs, path::Path, time::SystemTime};

pub struct FileCache<'a, T>(FileSystem<'a>, T)
where
    T: Resolver;

impl<'a, T> Resolver for FileCache<'a, T>
where
    T: Resolver,
{
    fn resolve<P>(&mut self, relative: P) -> Option<String>
    where
        P: AsRef<Path>,
    {
        let Self(cache, inner) = self;

        match (
            cache.last_modified(&relative),
            inner.last_modified(&relative),
        ) {
            (Some(cache_modified), Some(modified)) if cache_modified == modified => {
                return cache.resolve(&relative)
            }

            _ => (),
        }

        {
            let data = inner.resolve(&relative)?;
            let path = cache.0.join(&relative);

            fs::write(path.as_path(), data.as_str()).ok()?;

            Some(data)
        }
        .or(cache.resolve(&relative))
    }

    fn last_modified<P>(&self, relative: P) -> Option<SystemTime>
    where
        P: AsRef<Path>,
    {
        self.1.last_modified(relative)
    }
}

#[cfg(test)]
mod tests {
    use super::FileCache;
    use crate::resolve::{file_system::FileSystem, Resolver};
    use std::{fs, path::Path};
    use tempfile::tempdir;

    const TARGET_FILE: &str = "target_file.txt";
    const FILE_CONTENTS: &str = "what's in the box?!";

    #[test]
    fn resolve_from_source() {
        let cache_dir = tempdir().unwrap();

        let source_dir = tempdir().unwrap();
        let source = FileSystem(source_dir.path());
        source.write(TARGET_FILE, FILE_CONTENTS);

        let mut file_cache = FileCache(FileSystem(cache_dir.path()), source);

        assert_eq!(
            file_cache.resolve(Path::new(TARGET_FILE)),
            Some(FILE_CONTENTS.to_string())
        );
        assert_eq!(
            fs::read_to_string(cache_dir.path().join(TARGET_FILE)).unwrap(),
            FILE_CONTENTS.to_string()
        );
    }

    #[test]
    fn resolve_from_fresh_cache() {
        let source_dir = tempdir().unwrap();
        let source = FileSystem(source_dir.path());
        source.write(TARGET_FILE, FILE_CONTENTS);

        let cache_dir = tempdir().unwrap();
        let cache = FileSystem(cache_dir.path());
        cache.write(TARGET_FILE, FILE_CONTENTS);

        let mut file_cache = FileCache(cache, source);

        assert_eq!(
            file_cache.resolve(Path::new(TARGET_FILE)),
            Some(FILE_CONTENTS.to_string())
        );
    }

    #[test]
    fn resolve_from_stale_cache() {
        let cache_dir = tempdir().unwrap();
        let cache = FileSystem(cache_dir.path());
        cache.write(TARGET_FILE, "stale contents");

        let source_dir = tempdir().unwrap();
        let source = FileSystem(source_dir.path());
        source.write(TARGET_FILE, FILE_CONTENTS);

        let mut file_cache = FileCache(cache, FileSystem(source_dir.path()));

        assert_eq!(
            file_cache.resolve(Path::new(TARGET_FILE)),
            Some(FILE_CONTENTS.to_string())
        );
        assert_eq!(
            fs::read_to_string(cache_dir.path().join(TARGET_FILE)).unwrap(),
            FILE_CONTENTS.to_string()
        );
    }

    #[test]
    fn last_modified() {
        let cache_dir = tempdir().unwrap();

        let source_dir = tempdir().unwrap();
        let source = FileSystem(source_dir.path());
        source.write(TARGET_FILE, FILE_CONTENTS);

        let file_cache = FileCache(FileSystem(cache_dir.path()), source);

        assert!(file_cache.last_modified(Path::new(TARGET_FILE)).is_some());
    }
}
