mod context;
mod error;
#[cfg(test)]
mod fixture;
mod infer;
mod into_typed;
mod semantic;

pub use context::{Context, ModuleMap};
use error::Error;
use into_typed::Visitor;
use lang::{ast, NodeId};

/// analysis result with possible resolution errors
pub type Result<Value> = std::result::Result<Value, Vec<(NodeId, Error)>>;

pub fn analyze<Raw>(ctx: &Context, raw: Raw) -> Result<ast::typed::Program>
where
    Raw: ast::into_fragments::IntoFragments + into_typed::IntoTyped + Clone,
{
    let fragments = raw.clone().into_fragments();

    let weak = infer::weak::infer_types(&fragments);
    let strong = infer::strong::infer_types(ctx, weak)?;

    let typed = raw.into_typed(Visitor::new(&strong));

    semantic::analyze(ctx, typed)
}

#[cfg(test)]
mod tests {
    use crate::{fixture, Context, ModuleMap};
    use kore::{assert_eq, str};
    use lang::{
        ast,
        types::{Kind, Type},
        CanonicalId, Namespace, NamespaceId, NamespaceKind, Node, NodeId,
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
        let modules = ModuleMap::default();
        let ctx = Context {
            namespace: &Namespace(NamespaceKind::Internal, vec![str!("foo")]),
            ..Context::mock(&modules)
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
        let modules = ModuleMap::default();
        let ctx = Context {
            namespace: &Namespace(NamespaceKind::Internal, vec![str!("foo")]),
            ..Context::mock(&modules)
        };
        let raw = program(vec![], vec![fixture::type_alias::mock()]);

        assert_eq!(
            super::analyze(&ctx, raw),
            Ok(ast::meta::Program(ast::meta::Module(Node::mock(
                ast::Module::new(vec![], vec![fixture::type_alias::typed()]),
                ast::typed::Type(Type::Module(vec![(
                    str!("MyTypeAlias"),
                    Kind::Type,
                    Rc::new((CanonicalId::mock(0), fixture::type_alias::type_of()))
                )]))
            ))))
        );
    }

    #[test]
    fn constant() {
        let modules = ModuleMap::default();
        let ctx = Context {
            namespace: &Namespace(NamespaceKind::Internal, vec![str!("foo")]),
            ..Context::mock(&modules)
        };
        let raw = program(vec![], vec![fixture::constant::mock()]);

        assert_eq!(
            super::analyze(&ctx, raw),
            Ok(ast::meta::Program(ast::meta::Module(Node::mock(
                ast::Module::new(vec![], vec![fixture::constant::typed()]),
                ast::typed::Type(Type::Module(vec![(
                    str!("MY_CONSTANT"),
                    Kind::Value,
                    Rc::new((CanonicalId::mock(0), fixture::constant::type_of()))
                )]))
            ))))
        );
    }

    #[test]
    fn enumerated() {
        let modules = ModuleMap::default();
        let ctx = Context {
            namespace: &Namespace(NamespaceKind::Internal, vec![str!("foo")]),
            ..Context::mock(&modules)
        };
        let raw = program(vec![], vec![fixture::enumerated::mock()]);

        assert_eq!(
            super::analyze(&ctx, raw),
            Ok(ast::meta::Program(ast::meta::Module(Node::mock(
                ast::Module::new(vec![], vec![fixture::enumerated::typed()]),
                ast::typed::Type(Type::Module(vec![(
                    str!("MyEnum"),
                    Kind::Mixed,
                    Rc::new((CanonicalId::mock(2), fixture::enumerated::type_of()))
                )]))
            ))))
        );
    }

    #[ignore = "parameter inference not implemented"]
    #[test]
    fn function() {
        let modules = ModuleMap::default();
        let ctx = Context {
            namespace: &Namespace(NamespaceKind::Internal, vec![str!("foo")]),
            ..Context::mock(&modules)
        };
        let raw = program(vec![], vec![fixture::function::mock()]);

        assert_eq!(
            super::analyze(&ctx, raw),
            Ok(ast::meta::Program(ast::meta::Module(Node::mock(
                ast::Module::new(vec![], vec![fixture::function::typed()]),
                ast::typed::Type(Type::Module(vec![(
                    str!("MyEnum"),
                    Kind::Mixed,
                    Rc::new((CanonicalId::mock(0), fixture::function::type_of()))
                )]))
            ))))
        );
    }

    #[test]
    fn view() {
        let modules = ModuleMap::default();
        let ctx = Context {
            namespace: &Namespace(NamespaceKind::Internal, vec![str!("foo")]),
            ..Context::mock(&modules)
        };
        let raw = program(vec![], vec![fixture::view::mock()]);

        assert_eq!(
            super::analyze(&ctx, raw),
            Ok(ast::meta::Program(ast::meta::Module(Node::mock(
                ast::Module::new(vec![], vec![fixture::view::typed()]),
                ast::typed::Type(Type::Module(vec![(
                    str!("MyView"),
                    Kind::Value,
                    Rc::new((CanonicalId::mock(20), fixture::view::type_of()))
                )]))
            ))))
        );
    }

    #[test]
    fn module() {
        let modules = ModuleMap {
            keys: HashMap::from_iter(vec![(
                Namespace(NamespaceKind::Internal, vec![str!("theme")]),
                NamespaceId(1),
            )]),
            by_key: HashMap::from_iter(vec![(
                NamespaceId(1),
                (
                    CanonicalId(NamespaceId(1), NodeId(0)),
                    HashMap::from_iter(vec![
                        (
                            CanonicalId(NamespaceId(1), NodeId(0)),
                            Rc::new((
                                CanonicalId(NamespaceId(1), NodeId(0)),
                                ast::typed::Type(Type::Module(vec![(
                                    str!("PRIMARY"),
                                    Kind::Value,
                                    Rc::new((
                                        CanonicalId(NamespaceId(1), NodeId(1)),
                                        ast::typed::Type(Type::String),
                                    )),
                                )])),
                            )),
                        ),
                        (
                            CanonicalId(NamespaceId(1), NodeId(1)),
                            Rc::new((
                                CanonicalId(NamespaceId(1), NodeId(1)),
                                ast::typed::Type(Type::String),
                            )),
                        ),
                    ]),
                ),
            )]),
        };
        let ctx = Context {
            namespace: &Namespace(NamespaceKind::Internal, vec![str!("foo")]),
            ..Context::mock(&modules)
        };
        let raw = program(vec![], vec![fixture::module::mock()]);

        assert_eq!(
            super::analyze(&ctx, raw),
            Ok(ast::meta::Program(ast::meta::Module(Node::mock(
                ast::Module::new(vec![], vec![fixture::module::typed()]),
                ast::typed::Type(Type::Module(vec![(
                    str!("my_module"),
                    Kind::Mixed,
                    Rc::new((CanonicalId::mock(6), fixture::module::type_of()))
                )]))
            ))))
        );
    }
}
