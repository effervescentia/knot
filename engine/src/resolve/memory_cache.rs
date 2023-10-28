use super::Resolver;
use std::{collections::HashMap, path::Path, time::SystemTime};

pub struct MemoryCache<T>(HashMap<String, (String, SystemTime)>, T)
where
    T: Resolver;

impl<'a, T> Resolver for MemoryCache<T>
where
    T: Resolver,
{
    fn resolve<P>(&mut self, relative: P) -> Option<String>
    where
        P: AsRef<Path> + Copy,
    {
        let Self(ref mut cache, inner) = self;

        let key = || {
            relative
                .as_ref()
                .to_str()
                .expect("failed to turn relative path into a string")
                .to_string()
        };

        match (cache.get(&key()), inner.last_modified(relative)) {
            // cache is fresh
            (Some((data, cache_modified)), Some(ref modified)) if cache_modified == modified => {
                return Some(data.clone())
            }

            _ => (),
        }

        {
            let data = inner.resolve(&relative)?;
            let modified = inner.last_modified(&relative)?;

            cache.insert(key(), (data.clone(), modified));

            Some(data)
        }
        .or(cache.get(&key()).map(|(data, _)| data.clone()))
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
    use super::MemoryCache;
    use crate::{resolve::Resolver, FileSystem};
    use std::{collections::HashMap, path::Path, time::SystemTime};
    use tempfile::tempdir;

    const TARGET_FILE: &str = "target_file.txt";
    const FILE_CONTENTS: &str = "what's in the box?!";

    #[test]
    fn resolve_from_source() {
        let source_dir = tempdir().unwrap();
        let source = FileSystem(source_dir.path());
        source.write(TARGET_FILE, FILE_CONTENTS);

        let mut memory_cache = MemoryCache(HashMap::new(), source);

        assert_eq!(
            memory_cache.resolve(Path::new(TARGET_FILE)),
            Some(FILE_CONTENTS.to_string())
        );
        println!("{:?}", memory_cache.0);
        assert_eq!(memory_cache.0.get(""), None);
    }

    #[test]
    fn resolve_from_fresh_cache() {
        let source_dir = tempdir().unwrap();
        let source = FileSystem(source_dir.path());
        source.write(TARGET_FILE, FILE_CONTENTS);

        let mut cache = HashMap::new();
        cache.insert(
            TARGET_FILE.to_string(),
            (FILE_CONTENTS.to_string(), SystemTime::now()),
        );

        let mut memory_cache = MemoryCache(cache, FileSystem(source_dir.path()));

        assert_eq!(
            memory_cache.resolve(Path::new(TARGET_FILE)),
            Some(FILE_CONTENTS.to_string())
        );
    }

    #[test]
    fn resolve_from_stale_cache() {
        let mut cache = HashMap::new();
        cache.insert(
            TARGET_FILE.to_string(),
            (FILE_CONTENTS.to_string(), SystemTime::now()),
        );

        let source_dir = tempdir().unwrap();
        let source = FileSystem(source_dir.path());
        source.write(TARGET_FILE, FILE_CONTENTS);

        let mut memory_cache = MemoryCache(cache, FileSystem(source_dir.path()));

        assert_eq!(
            memory_cache.resolve(Path::new(TARGET_FILE)),
            Some(FILE_CONTENTS.to_string())
        );
        assert_eq!(
            memory_cache.0.get(TARGET_FILE).unwrap().0,
            FILE_CONTENTS.to_string()
        );
    }

    #[test]
    fn last_modified() {
        let dir = tempdir().unwrap();
        let file_system = FileSystem(dir.path());
        file_system.write(TARGET_FILE, FILE_CONTENTS);
        let memory_cache = MemoryCache(HashMap::new(), file_system);

        assert!(memory_cache.last_modified(Path::new(TARGET_FILE)).is_some());
    }
}
