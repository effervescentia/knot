use super::{base::Base, Ast, FromPaths, IsVerbose, Module};
use crate::{report, Context, ExecutionError, Link};
use kore::graph::Graph;
use kore::{color::Highlight, Incrementor};
use lang::{ModuleId, Namespace};
use std::{
    cell::RefCell,
    ops::{Deref, DerefMut},
    rc::Rc,
};

#[derive(Clone)]
pub struct Parsed(pub Base<()>, pub Rc<RefCell<Incrementor>>);

impl Parsed {
    pub fn new(verbose: bool) -> Self {
        Self(Base::new(verbose), Default::default())
    }

    pub fn incrementor(&self) -> Rc<RefCell<Incrementor>> {
        Rc::clone(&self.1)
    }

    pub fn to_import_graph(&self) -> Graph<ModuleId> {
        self.internal_modules()
            .fold(Graph::new(), |mut graph, (_, x)| {
                graph.upsert_node(x.id);
                graph
            })
    }

    pub fn link_modules<R>(&self, context: &mut Context<R>) -> crate::Internal<Graph<ModuleId>> {
        self.internal_modules()
            .try_fold(self.to_import_graph(), |mut acc, (link, module)| {
                let links = module.ast.to_links(link);

                for x in &links {
                    if let Some(x) = self.get_id_by_link(x) {
                        acc.add_edge(&module.id, x);
                    } else if matches!(x.0, Namespace::Library(_)) {
                        continue;
                    } else {
                        context.raise(ExecutionError::UnregisteredModule(x.clone()))?;
                    }
                }

                Ok(acc)
            })
    }

    pub fn register_source(&mut self, link: Link, text: String, ast: Ast<()>) {
        let incrementor = self.incrementor();
        let module = Module {
            id: ModuleId(incrementor.borrow_mut().deref_mut().increment()),
            text,
            ast,
        };

        self.0.register_module(link, module);
    }

    pub fn report_from_entry(&self) {
        if self.is_verbose() {
            eprintln!(
                "\u{1f440} parsed {} module(s) by traversing the import graph",
                self.internal_modules().count().to_string().focus()
            );
        }
    }

    pub fn report_from_glob(&self) {
        if self.is_verbose() {
            eprintln!(
                "\u{1f440} parsed {} matching module(s)",
                self.internal_modules().count().to_string().focus()
            );
        }
    }
}

impl Deref for Parsed {
    type Target = Base<()>;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for Parsed {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

impl From<FromPaths> for Parsed {
    fn from(value: FromPaths) -> Self {
        Self::new(value.verbose)
    }
}

impl report::Enrich for Parsed {
    fn enrich(&self, root_dir: String, failure: report::Failure) -> report::Report {
        self.0.enrich(root_dir, failure)
    }
}
