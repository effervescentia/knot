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

            fs::write(path.as_path(), &data).ok()?;

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
    use crate::{resolve::Resolver, FileSystem};
    use std::{fs, path::Path, thread::sleep, time::Duration};
    use tempfile::tempdir;

    const TARGET_FILE: &str = "target_file.txt";
    const STALE_DATA: &str = "stale data";
    const FRESH_DATA: &str = "fresh data";

    #[test]
    fn resolve_from_source() {
        let cache_dir = tempdir().unwrap();

        let source_dir = tempdir().unwrap();
        let source = FileSystem(source_dir.path());
        source.write(TARGET_FILE, FRESH_DATA);

        let mut file_cache = FileCache(FileSystem(cache_dir.path()), source);

        assert_eq!(
            file_cache.resolve(Path::new(TARGET_FILE)),
            Some(FRESH_DATA.to_owned()),
            "data should come from the source file"
        );
        assert_eq!(
            fs::read_to_string(cache_dir.path().join(TARGET_FILE)).unwrap(),
            FRESH_DATA.to_owned(),
            "file should be copied to the cache"
        );
    }

    #[test]
    fn resolve_from_fresh_cache() {
        let source_dir = tempdir().unwrap();
        let source = FileSystem(source_dir.path());
        source.write(TARGET_FILE, STALE_DATA);

        sleep(Duration::from_millis(50));

        let cache_dir = tempdir().unwrap();
        let cache = FileSystem(cache_dir.path());
        cache.write(TARGET_FILE, FRESH_DATA);

        let mut file_cache = FileCache(cache, source);

        assert_eq!(
            file_cache.resolve(Path::new(TARGET_FILE)),
            Some(FRESH_DATA.to_owned()),
            "data should come from the cached file"
        );
    }

    #[test]
    fn resolve_from_stale_cache() {
        let cache_dir = tempdir().unwrap();
        let cache = FileSystem(cache_dir.path());
        cache.write(TARGET_FILE, STALE_DATA);

        sleep(Duration::from_millis(50));

        let source_dir = tempdir().unwrap();
        let source = FileSystem(source_dir.path());
        source.write(TARGET_FILE, FRESH_DATA);

        let mut file_cache = FileCache(cache, FileSystem(source_dir.path()));

        assert_eq!(
            file_cache.resolve(Path::new(TARGET_FILE)),
            Some(FRESH_DATA.to_owned()),
            "data should come from the source file"
        );
        assert_eq!(
            fs::read_to_string(cache_dir.path().join(TARGET_FILE)).unwrap(),
            FRESH_DATA.to_owned(),
            "cache file should be overwritten"
        );
    }

    #[test]
    fn last_modified() {
        let cache_dir = tempdir().unwrap();

        let source_dir = tempdir().unwrap();
        let source = FileSystem(source_dir.path());
        source.write(TARGET_FILE, FRESH_DATA);

        let file_cache = FileCache(FileSystem(cache_dir.path()), source);

        assert!(
            file_cache.last_modified(Path::new(TARGET_FILE)).is_some(),
            "should get the last modified date"
        );
    }
}
