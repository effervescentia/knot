mod error;
mod infer;
mod into_typed;

use error::ResolveError;
use into_typed::Visitor;
use lang::{ast, ModuleReference, NodeId};
use std::collections::HashMap;

pub type Result<Value> = std::result::Result<Value, Vec<(NodeId, ResolveError)>>;

pub struct Context<'a> {
    pub namespace: &'a ModuleReference,
    pub modules: &'a HashMap<ModuleReference, &'a ast::typed::Type>,
}

pub fn analyze<Raw>(ctx: &Context, raw: Raw) -> Result<ast::typed::Program>
where
    Raw: ast::into_fragments::IntoFragments + into_typed::IntoTyped + Clone,
{
    let fragments = raw.clone().into_fragments();

    let weak = infer::weak::infer_types(&fragments);
    let strong = infer::strong::infer_types(ctx, weak)?;

    let typed = raw.into_typed(Visitor::new(strong));

    // do semantic analysis

    Ok(typed)
}

#[cfg(test)]
mod tests {
    use crate::Context;
    use kore::{assert_eq, str};
    use lang::{
        ast,
        test::fixture,
        types::{Kind, Type},
        ModuleReference, ModuleScope, Node,
    };
    use std::{collections::HashMap, rc::Rc};

    fn program(
        imports: Vec<ast::raw::Import>,
        declarations: Vec<ast::raw::Declaration>,
    ) -> ast::raw::Program {
        ast::meta::Program(ast::meta::Module::mock(ast::Module::new(
            imports,
            declarations,
        )))
    }

    #[test]
    fn empty_module() {
        let ctx = Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let raw = program(vec![], vec![]);

        assert_eq!(
            super::analyze(&ctx, raw),
            Ok(ast::meta::Program(ast::meta::Module(Node::mock(
                ast::Module::new(vec![], vec![]),
                ast::typed::Type(Type::Module(vec![]))
            ))))
        );
    }

    #[test]
    fn type_alias() {
        let ctx = Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let raw = program(vec![], vec![fixture::type_alias::mock()]);

        assert_eq!(
            super::analyze(&ctx, raw),
            Ok(ast::meta::Program(ast::meta::Module(Node::mock(
                ast::Module::new(vec![], vec![fixture::type_alias::typed()]),
                ast::typed::Type(Type::Module(vec![(
                    str!("MyTypeAlias"),
                    Kind::Type,
                    Rc::new(fixture::type_alias::type_of())
                )]))
            ))))
        );
    }

    #[test]
    fn constant() {
        let ctx = Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let raw = program(vec![], vec![fixture::constant::mock()]);

        assert_eq!(
            super::analyze(&ctx, raw),
            Ok(ast::meta::Program(ast::meta::Module(Node::mock(
                ast::Module::new(vec![], vec![fixture::constant::typed()]),
                ast::typed::Type(Type::Module(vec![(
                    str!("MY_CONSTANT"),
                    Kind::Value,
                    Rc::new(fixture::constant::type_of())
                )]))
            ))))
        );
    }

    #[test]
    fn enumerated() {
        let ctx = Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let raw = program(vec![], vec![fixture::enumerated::mock()]);

        assert_eq!(
            super::analyze(&ctx, raw),
            Ok(ast::meta::Program(ast::meta::Module(Node::mock(
                ast::Module::new(vec![], vec![fixture::enumerated::typed()]),
                ast::typed::Type(Type::Module(vec![(
                    str!("MyEnum"),
                    Kind::Mixed,
                    Rc::new(fixture::enumerated::type_of())
                )]))
            ))))
        );
    }

    #[ignore = "parameter inference not implemented"]
    #[test]
    fn function() {
        let ctx = Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let raw = program(vec![], vec![fixture::function::mock()]);

        assert_eq!(
            super::analyze(&ctx, raw),
            Ok(ast::meta::Program(ast::meta::Module(Node::mock(
                ast::Module::new(vec![], vec![fixture::function::typed()]),
                ast::typed::Type(Type::Module(vec![(
                    str!("MyEnum"),
                    Kind::Mixed,
                    Rc::new(fixture::function::type_of())
                )]))
            ))))
        );
    }

    #[ignore = "reason"]
    #[test]
    fn view() {
        let ctx = Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let raw = program(vec![], vec![fixture::view::mock()]);

        assert_eq!(
            super::analyze(&ctx, raw),
            Ok(ast::meta::Program(ast::meta::Module(Node::mock(
                ast::Module::new(vec![], vec![fixture::view::typed()]),
                ast::typed::Type(Type::Module(vec![(
                    str!("MyEnum"),
                    Kind::Mixed,
                    Rc::new(fixture::view::type_of())
                )]))
            ))))
        );
    }

    #[ignore = "import inference not implemented"]
    #[test]
    fn module() {
        let ctx = Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let raw = program(vec![], vec![fixture::module::mock()]);

        assert_eq!(
            super::analyze(&ctx, raw),
            Ok(ast::meta::Program(ast::meta::Module(Node::mock(
                ast::Module::new(vec![], vec![fixture::module::typed()]),
                ast::typed::Type(Type::Module(vec![(
                    str!("my_module"),
                    Kind::Mixed,
                    Rc::new(fixture::module::type_of())
                )]))
            ))))
        );
    }
}
