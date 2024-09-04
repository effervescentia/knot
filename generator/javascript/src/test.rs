use crate::{resolve::Resolver, Module, Options};
use kore::internal;

#[derive(Clone, Copy, Default)]
pub struct MockLibrary;

impl internal::PlatformLibrary for MockLibrary {
    fn text(&self) -> &str {
        "<mock_library>"
    }

    fn module(&self) -> Option<&'static str> {
        None
    }
}

impl From<MockLibrary> for internal::Library {
    fn from(_: MockLibrary) -> Self {
        Self::Std
    }
}

impl From<internal::Library> for MockLibrary {
    fn from(_: internal::Library) -> Self {
        Self
    }
}

pub const MOCK_OPTIONS: Options<MockLibrary> = Options {
    mode: internal::Mode::Production,
    resolver: Resolver::new(Module::ESM),
};
