mod file_cache;
mod file_system;
mod memory_cache;

pub use file_cache::FileCache;
pub use file_system::FileSystem;
pub use memory_cache::MemoryCache;
use std::{path::Path, time::SystemTime};

pub trait Resolver {
    fn resolve<P>(&mut self, relative: P) -> Option<String>
    where
        P: AsRef<Path> + Copy;

    fn last_modified<P>(&self, relative: P) -> Option<SystemTime>
    where
        P: AsRef<Path>;
}
