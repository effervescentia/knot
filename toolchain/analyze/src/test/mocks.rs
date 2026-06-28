use crate::{
    analyze,
    context::{AmbientMap, TypeMap},
    Context, ModuleMap, Result,
};
use lang::ast;

#[derive(Debug, Default)]
pub struct Mock {
    pub ambient: AmbientMap,
    pub modules: ModuleMap,
}

impl Mock {
    pub const fn context(&self) -> Context {
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
    pub fn modules(x: &super::ModuleMap) -> super::ModuleMap {
        x.clone()
    }

    pub fn ambient(x: &super::AmbientMap) -> super::AmbientMap {
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
