use crate::{context::AmbientMap, Context, ModuleMap};

#[derive(Debug, Default)]
pub struct Mock {
    pub ambient: AmbientMap,
    pub modules: ModuleMap,
}

impl Mock {
    pub const fn context(&self) -> Context {
        Context::mock(&self.modules, &self.ambient)
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
