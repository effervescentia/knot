use crate::{
    analyze,
    context::{AmbientMap, LibraryMap, TypeMap},
    Context, ModuleMap, Result,
};
use kore::Serializable;
use lang::ast;
use std::str::FromStr;

#[derive(Clone, Copy, Debug, Default, Eq, Hash, PartialEq)]
pub struct MockLibrary;

impl Serializable for MockLibrary {}

impl FromStr for MockLibrary {
    type Err = ();

    fn from_str(_: &str) -> std::result::Result<Self, Self::Err> {
        Err(())
    }
}

#[derive(Debug, Default)]
pub struct Mock {
    pub ambient: AmbientMap,
    pub modules: ModuleMap<MockLibrary>,
    pub libraries: LibraryMap<MockLibrary>,
}

impl Mock {
    pub const fn context(&self) -> Context<MockLibrary> {
        Context::mock(&self.modules, &self.ambient)
    }

    #[allow(clippy::unwrap_in_result)]
    #[cfg(feature = "test")]
    pub fn parse_and_analyze(self, source: &str) -> Result<(ast::typed::Program, TypeMap)> {
        let ctx = self.context();
        let (raw, _) = parse::program::parse(source).ok().unwrap();

        analyze(&ctx, raw)
    }
}

pub mod setters {
    use kore::Serializable;

    pub fn modules<Library>(x: &super::ModuleMap<Library>) -> super::ModuleMap<Library>
    where
        Library: Serializable,
    {
        x.clone()
    }

    pub fn ambient(x: &super::AmbientMap) -> super::AmbientMap {
        x.clone()
    }

    pub fn libraries<Library>(x: &super::LibraryMap<Library>) -> super::LibraryMap<Library>
    where
        Library: Serializable,
    {
        x.clone()
    }
}

#[macro_export]
macro_rules! analyze_mock {
    ($($field:ident = $value:expr),* $(,)?) => {
        ($crate::test::mocks::Mock {
            $(
                $field: $crate::test::mocks::setters::$field($value),
            )*
            ..Default::default()
        })
    }
}
