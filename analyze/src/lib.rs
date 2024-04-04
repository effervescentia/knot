mod context;
mod error;
#[cfg(test)]
mod fixture;
mod infer;
mod into_typed;
mod semantic;

pub use context::{Context, ModuleMap, TypeMap};
use error::Error;
use lang::{ast, NodeId};

/// analysis result with possible resolution errors
pub type Result<Value> = std::result::Result<Value, Vec<(NodeId, Error)>>;

pub fn analyze<Raw>(ctx: &Context, raw: Raw) -> Result<(ast::typed::Program, TypeMap)>
where
    Raw: ast::into_fragments::IntoFragments<NodeId> + into_typed::IntoTyped + Clone,
{
    let fragments = raw.clone().into_fragments();

    let weak = infer::weak::infer_types(&fragments);
    let strong = infer::strong::infer_types(ctx, weak)?;

    let mut typed = raw.into_typed(ctx, &strong);
    let types = strong.canonicalize(ctx);

    typed = semantic::analyze(ctx, fragments, typed)?;

    Ok((typed, types))
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
            Ok((
                ast::meta::Program(ast::meta::Module(Node::mock(
                    ast::Module::new(vec![], vec![]),
                    (CanonicalId::mock(0), ast::typed::Type(Type::Module(vec![])))
                ))),
                HashMap::from_iter(vec![(
                    CanonicalId::mock(0),
                    Rc::new((CanonicalId::mock(0), ast::typed::Type(Type::Module(vec![]))))
                )])
            ))
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
            Ok((
                ast::meta::Program(ast::meta::Module(Node::mock(
                    ast::Module::new(vec![], vec![fixture::type_alias::typed()]),
                    (
                        CanonicalId::mock(2),
                        ast::typed::Type(Type::Module(vec![(
                            str!("MyTypeAlias"),
                            Kind::Type,
                            Rc::new((CanonicalId::mock(0), fixture::type_alias::type_of()))
                        )]))
                    )
                ))),
                HashMap::from_iter(vec![
                    (
                        CanonicalId::mock(0),
                        Rc::new((CanonicalId::mock(0), fixture::type_alias::type_of()))
                    ),
                    (
                        CanonicalId::mock(1),
                        Rc::new((CanonicalId::mock(0), fixture::type_alias::type_of()))
                    ),
                    (
                        CanonicalId::mock(2),
                        Rc::new((
                            CanonicalId::mock(2),
                            ast::typed::Type(Type::Module(vec![(
                                str!("MyTypeAlias"),
                                Kind::Type,
                                Rc::new((CanonicalId::mock(0), fixture::type_alias::type_of()))
                            )]))
                        ))
                    )
                ])
            ))
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
            Ok((
                ast::meta::Program(ast::meta::Module(Node::mock(
                    ast::Module::new(vec![], vec![fixture::constant::typed()]),
                    (
                        CanonicalId::mock(3),
                        ast::typed::Type(Type::Module(vec![(
                            str!("MY_CONSTANT"),
                            Kind::Value,
                            Rc::new((CanonicalId::mock(0), fixture::constant::type_of()))
                        )]))
                    )
                ))),
                HashMap::from_iter(vec![
                    (
                        CanonicalId::mock(0),
                        Rc::new((CanonicalId::mock(0), fixture::constant::type_of()))
                    ),
                    (
                        CanonicalId::mock(1),
                        Rc::new((CanonicalId::mock(1), fixture::constant::type_of()))
                    ),
                    (
                        CanonicalId::mock(2),
                        Rc::new((CanonicalId::mock(0), fixture::constant::type_of()))
                    ),
                    (
                        CanonicalId::mock(3),
                        Rc::new((
                            CanonicalId::mock(3),
                            ast::typed::Type(Type::Module(vec![(
                                str!("MY_CONSTANT"),
                                Kind::Value,
                                Rc::new((CanonicalId::mock(0), fixture::constant::type_of()))
                            )]))
                        ))
                    )
                ])
            ))
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
            Ok((
                ast::meta::Program(ast::meta::Module(Node::mock(
                    ast::Module::new(vec![], vec![fixture::enumerated::typed()]),
                    (
                        CanonicalId::mock(3),
                        ast::typed::Type(Type::Module(vec![(
                            str!("MyEnum"),
                            Kind::Mixed,
                            Rc::new((CanonicalId::mock(2), fixture::enumerated::type_of()))
                        )]))
                    )
                ))),
                HashMap::from_iter(vec![
                    (
                        CanonicalId::mock(0),
                        Rc::new((CanonicalId::mock(0), ast::typed::Type(Type::Boolean)))
                    ),
                    (
                        CanonicalId::mock(1),
                        Rc::new((CanonicalId::mock(1), ast::typed::Type(Type::Style)))
                    ),
                    (
                        CanonicalId::mock(2),
                        Rc::new((CanonicalId::mock(2), fixture::enumerated::type_of()))
                    ),
                    (
                        CanonicalId::mock(3),
                        Rc::new((
                            CanonicalId::mock(3),
                            ast::typed::Type(Type::Module(vec![(
                                str!("MyEnum"),
                                Kind::Mixed,
                                Rc::new((CanonicalId::mock(2), fixture::enumerated::type_of()))
                            )]))
                        ))
                    )
                ])
            ))
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
            Ok((
                ast::meta::Program(ast::meta::Module(Node::mock(
                    ast::Module::new(vec![], vec![fixture::function::typed()]),
                    (
                        CanonicalId::mock(7),
                        ast::typed::Type(Type::Module(vec![(
                            str!("MyEnum"),
                            Kind::Mixed,
                            Rc::new((CanonicalId::mock(0), fixture::function::type_of()))
                        )]))
                    )
                ))),
                HashMap::from_iter(vec![])
            ))
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
            Ok((
                ast::meta::Program(ast::meta::Module(Node::mock(
                    ast::Module::new(vec![], vec![fixture::view::typed()]),
                    (
                        CanonicalId::mock(21),
                        ast::typed::Type(Type::Module(vec![(
                            str!("MyView"),
                            Kind::Value,
                            Rc::new((CanonicalId::mock(20), fixture::view::type_of()))
                        )]))
                    )
                ))),
                HashMap::from_iter(vec![
                    (
                        CanonicalId::mock(0),
                        Rc::new((CanonicalId::mock(0), ast::typed::Type(Type::Element)))
                    ),
                    (
                        CanonicalId::mock(1),
                        Rc::new((CanonicalId::mock(1), ast::typed::Type(Type::Element)))
                    ),
                    (
                        CanonicalId::mock(2),
                        Rc::new((CanonicalId::mock(1), ast::typed::Type(Type::Element)))
                    ),
                    (
                        CanonicalId::mock(3),
                        Rc::new((CanonicalId::mock(0), ast::typed::Type(Type::Element)))
                    ),
                    (
                        CanonicalId::mock(4),
                        Rc::new((CanonicalId::mock(4), ast::typed::Type(Type::Integer)))
                    ),
                    (
                        CanonicalId::mock(5),
                        Rc::new((CanonicalId::mock(5), ast::typed::Type(Type::Float)))
                    ),
                    (
                        CanonicalId::mock(6),
                        Rc::new((CanonicalId::mock(6), ast::typed::Type(Type::Float)))
                    ),
                    (
                        CanonicalId::mock(7),
                        Rc::new((CanonicalId::mock(7), ast::typed::Type(Type::Nil)))
                    ),
                    (
                        CanonicalId::mock(8),
                        Rc::new((CanonicalId::mock(8), ast::typed::Type(Type::String)))
                    ),
                    (
                        CanonicalId::mock(9),
                        Rc::new((CanonicalId::mock(9), ast::typed::Type(Type::Element)))
                    ),
                    (
                        CanonicalId::mock(10),
                        Rc::new((CanonicalId::mock(6), ast::typed::Type(Type::Float)))
                    ),
                    (
                        CanonicalId::mock(11),
                        Rc::new((CanonicalId::mock(6), ast::typed::Type(Type::Float)))
                    ),
                    (
                        CanonicalId::mock(12),
                        Rc::new((CanonicalId::mock(12), ast::typed::Type(Type::String)))
                    ),
                    (
                        CanonicalId::mock(13),
                        Rc::new((CanonicalId::mock(0), ast::typed::Type(Type::Element)))
                    ),
                    (
                        CanonicalId::mock(14),
                        Rc::new((CanonicalId::mock(0), ast::typed::Type(Type::Element)))
                    ),
                    (
                        CanonicalId::mock(15),
                        Rc::new((CanonicalId::mock(15), ast::typed::Type(Type::Element)))
                    ),
                    (
                        CanonicalId::mock(16),
                        Rc::new((CanonicalId::mock(16), ast::typed::Type(Type::Element)))
                    ),
                    (
                        CanonicalId::mock(17),
                        Rc::new((CanonicalId::mock(16), ast::typed::Type(Type::Element)))
                    ),
                    (
                        CanonicalId::mock(18),
                        Rc::new((CanonicalId::mock(16), ast::typed::Type(Type::Element)))
                    ),
                    (
                        CanonicalId::mock(19),
                        Rc::new((CanonicalId::mock(16), ast::typed::Type(Type::Element)))
                    ),
                    (
                        CanonicalId::mock(20),
                        Rc::new((CanonicalId::mock(20), fixture::view::type_of()))
                    ),
                    (
                        CanonicalId::mock(21),
                        Rc::new((
                            CanonicalId::mock(21),
                            ast::typed::Type(Type::Module(vec![(
                                str!("MyView"),
                                Kind::Value,
                                Rc::new((CanonicalId::mock(20), fixture::view::type_of()))
                            )]))
                        ))
                    ),
                ])
            ))
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
            Ok((
                ast::meta::Program(ast::meta::Module(Node::mock(
                    ast::Module::new(vec![], vec![fixture::module::typed()]),
                    (
                        CanonicalId::mock(8),
                        ast::typed::Type(Type::Module(vec![(
                            str!("my_module"),
                            Kind::Mixed,
                            Rc::new((CanonicalId::mock(6), fixture::module::type_of()))
                        )]))
                    )
                ))),
                HashMap::from_iter(vec![
                    (
                        CanonicalId::mock(0),
                        Rc::new((
                            CanonicalId(NamespaceId(1), NodeId(0)),
                            ast::typed::Type(Type::Module(vec![(
                                str!("PRIMARY"),
                                Kind::Value,
                                Rc::new((
                                    CanonicalId(NamespaceId(1), NodeId(1)),
                                    ast::typed::Type(Type::String),
                                )),
                            )]))
                        ))
                    ),
                    (
                        CanonicalId::mock(1),
                        Rc::new((
                            CanonicalId(NamespaceId(1), NodeId(0)),
                            ast::typed::Type(Type::Module(vec![(
                                str!("PRIMARY"),
                                Kind::Value,
                                Rc::new((
                                    CanonicalId(NamespaceId(1), NodeId(1)),
                                    ast::typed::Type(Type::String),
                                )),
                            )]))
                        ))
                    ),
                    (
                        CanonicalId::mock(2),
                        Rc::new((
                            CanonicalId(NamespaceId(1), NodeId(1)),
                            ast::typed::Type(Type::String)
                        ))
                    ),
                    (
                        CanonicalId::mock(3),
                        Rc::new((CanonicalId::mock(3), ast::typed::Type(Type::String)))
                    ),
                    (
                        CanonicalId::mock(4),
                        Rc::new((CanonicalId::mock(4), ast::typed::Type(Type::Style)))
                    ),
                    (
                        CanonicalId::mock(5),
                        Rc::new((CanonicalId::mock(4), ast::typed::Type(Type::Style)))
                    ),
                    (
                        CanonicalId::mock(6),
                        Rc::new((CanonicalId::mock(6), fixture::module::type_of()))
                    ),
                    (
                        CanonicalId::mock(7),
                        Rc::new((CanonicalId::mock(6), fixture::module::type_of()))
                    ),
                    (
                        CanonicalId::mock(8),
                        Rc::new((
                            CanonicalId::mock(8),
                            ast::typed::Type(Type::Module(vec![(
                                str!("my_module"),
                                Kind::Mixed,
                                Rc::new((CanonicalId::mock(6), fixture::module::type_of()))
                            )]))
                        ))
                    ),
                ])
            ))
        );
    }
}
