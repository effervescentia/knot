use super::data::{Output, Result, Strong, Type};
use crate::{
    error::ResolveError,
    infer::{weak, BindingMap, NodeDescriptor},
};
use kore::invariant;
use lang::{
    ast,
    types::{self, Enumerated, Kind},
    FragmentMap, NodeId,
};
use std::{cell::OnceCell, collections::BTreeMap, rc::Rc};

pub enum ResolvedType<'a> {
    Local(&'a types::Type<NodeId>),
    Remote(Rc<ast::typed::Type>),
}

// impl<'a> ResolvedType<'a> {
//     pub fn to_shape(&self) -> types::Type<()> {
//         match self {
//             ResolvedType::Local(x) => x.to_shape(),
//             ResolvedType::Remote(x) => x.0.to_shape(),
//         }
//     }
// }

/// type resolved from the `State` during inference
type ResolvedResult<'a> = std::result::Result<ResolvedType<'a>, &'a ResolveError>;

type Warning<'a> = (&'a NodeDescriptor, String);

#[derive(Debug, PartialEq)]
pub struct State<'a> {
    pub fragments: &'a FragmentMap,

    pub bindings: BindingMap,

    pub nodes: Vec<NodeDescriptor>,

    pub types: BTreeMap<NodeId, Strong>,

    pub warnings: Vec<Warning<'a>>,
}

impl<'a> State<'a> {
    pub fn from_weak(mut weak: weak::Result<'a>) -> Self {
        let nodes = weak.build_descriptors();

        Self {
            fragments: weak.fragments,
            bindings: weak.bindings,
            nodes,
            types: Default::default(),
            warnings: Default::default(),
        }
    }

    pub fn next(prev: Self) -> (Vec<NodeDescriptor>, Self) {
        (
            prev.nodes,
            Self {
                nodes: Default::default(),
                ..prev
            },
        )
    }

    pub fn is_done(&self) -> bool {
        self.nodes.is_empty()
    }

    pub fn get_type(
        &self,
        id: &NodeId,
        allowed_kind: &Kind,
    ) -> Option<&std::result::Result<Type, ResolveError>> {
        self.types.get(id).and_then(|(kind, strong)| {
            if !allowed_kind.can_accept(kind) {
                return None;
            }

            Some(strong)
        })
    }

    pub fn resolve(&self, id: &NodeId, allowed_kind: &Kind) -> Option<ResolvedResult> {
        self.get_type(id, allowed_kind)
            .and_then(|strong| match strong {
                Ok(Type::Local(local)) => Some(Ok(ResolvedType::Local(local))),

                Ok(Type::Remote(remote)) => Some(Ok(ResolvedType::Remote(Rc::clone(remote)))),

                Ok(Type::Inherit(from_id)) => self.resolve_any(from_id),

                Err(err) => Some(Err(err)),
            })
    }

    pub fn resolve_value(&self, id: &NodeId) -> Option<ResolvedResult> {
        self.resolve(id, &Kind::Value)
    }

    pub fn resolve_any(&self, id: &NodeId) -> Option<ResolvedResult> {
        self.resolve(id, &Kind::Mixed)
    }

    fn finalize_type(x: types::Type<NodeId>, output: &Output) -> Rc<ast::typed::Type> {
        let get_type = |id| {
            Rc::clone(
                output
                    .types
                    .get(id)
                    .and_then(OnceCell::get)
                    .unwrap_or_else(|| invariant!("type not found")),
            )
        };

        match x {
            types::Type::Nil => Rc::new(ast::typed::Type(types::Type::Nil)),
            types::Type::Boolean => Rc::new(ast::typed::Type(types::Type::Boolean)),
            types::Type::Integer => Rc::new(ast::typed::Type(types::Type::Integer)),
            types::Type::Float => Rc::new(ast::typed::Type(types::Type::Float)),
            types::Type::String => Rc::new(ast::typed::Type(types::Type::String)),
            types::Type::Style => Rc::new(ast::typed::Type(types::Type::Style)),
            types::Type::Element => Rc::new(ast::typed::Type(types::Type::Element)),

            types::Type::Enumerated(x) => {
                Rc::new(ast::typed::Type(types::Type::Enumerated(match x {
                    Enumerated::Declaration(variants) => Enumerated::Declaration(
                        variants
                            .iter()
                            .map(|(name, xs)| (name.clone(), xs.iter().map(get_type).collect()))
                            .collect(),
                    ),

                    Enumerated::Variant(parameters, instance) => Enumerated::Variant(
                        parameters.iter().map(get_type).collect(),
                        get_type(&instance),
                    ),

                    Enumerated::Instance(x) => Enumerated::Instance(get_type(&x)),
                })))
            }

            types::Type::Function(parameters, x) => Rc::new(ast::typed::Type(
                types::Type::Function(parameters.iter().map(get_type).collect(), get_type(&x)),
            )),

            types::Type::View(parameters) => Rc::new(ast::typed::Type(types::Type::View(
                parameters.iter().map(get_type).collect(),
            ))),

            types::Type::Module(declarations) => Rc::new(ast::typed::Type(types::Type::Module(
                declarations
                    .iter()
                    .map(|(name, kind, x)| (name.clone(), *kind, get_type(x)))
                    .collect(),
            ))),
        }
    }

    pub fn into_result(self) -> Result {
        let output = Output::new(self.types.keys());
        let mut errors = vec![];

        let get_cell = |id| {
            output
                .types
                .get(&id)
                .unwrap_or_else(|| invariant!("no cell exists to store type"))
        };

        for (id, (_, x)) in self.types {
            match x {
                Ok(Type::Local(x)) => {
                    let cell = get_cell(id);

                    cell.set(State::finalize_type(x, &output)).ok();
                }

                Ok(Type::Inherit(from_id)) => {
                    let cell = get_cell(id);
                    let from_cell = get_cell(from_id);

                    cell.set(Rc::clone(
                        from_cell
                            .get()
                            .unwrap_or_else(|| invariant!("inherited cell is empty")),
                    ))
                    .ok();
                }

                Ok(Type::Remote(x)) => {
                    get_cell(id).set(Rc::clone(&x)).ok();
                }

                Err(err) => errors.push((id, err)),
            }
        }

        if errors.is_empty() {
            Ok(output)
        } else {
            Err(errors)
        }
    }
}
