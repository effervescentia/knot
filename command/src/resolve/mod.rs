mod file_cache;
mod file_system;
mod memory_cache;

pub use file_cache::FileCache;
pub use file_system::FileSystem;
pub use memory_cache::MemoryCache;
use std::{
    path::{Path, PathBuf},
    time::SystemTime,
};

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
enum ModuleSource {
    Internal,
    External(String),
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct ModuleRef(ModuleSource, Vec<String>);

impl ModuleRef {
    pub fn to_path(&self) -> PathBuf {
        let Self(source, module_path) = self;
        let mut file_path = PathBuf::new();

        match source {
            ModuleSource::External(_namespace) => todo!(),

            ModuleSource::Internal => (),
        }

        file_path.extend(module_path.iter().map(Path::new).collect::<Vec<_>>());

        file_path
    }
}

pub trait Resolver {
    fn resolve<P>(&mut self, relative: P) -> Option<String>
    where
        P: AsRef<Path> + Copy;

    fn last_modified<P>(&self, relative: P) -> Option<SystemTime>
    where
        P: AsRef<Path>;
}
