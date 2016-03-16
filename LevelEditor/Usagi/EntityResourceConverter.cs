using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel.Composition;
using Sce.Atf;
using LevelEditorCore;
using System.Reflection;
using Sce.Atf.Dom;
using RenderingInterop;
using LevelEditor;
using Sce.Atf.Adaptation;
using LevelEditor.DomNodeAdapters;

namespace Vitei.RenderingInteropUsagi
{
    [Export(typeof(IResourceConverter))]
    [PartCreationPolicy(CreationPolicy.Shared)]
    public class EntityResourceConverter : IResourceConverter
    {
        IGameObject IResourceConverter.Convert(IResource resource)
        {
            if (resource == null) return null;

            IGameObject gob = null;
            if (resource.Type == EntityResolver.ResourceType)
            {
                /*
                // create nodes here; we'll do our own thing later, for now
                // just ape what ResourceConverter does.

                // ...USING RUNTIME REFLECTION! :D

                // okay, in my defense this seemed like something that would
                // have worked. for future reference: you can't tag locators
                // with anything other than models. if you try, it'll just get
                // cast to a model and then the editor falls over (see
                // Locator::Update(), around line 100 in Locator.cpp).
                Type locType = Assembly.GetEntryAssembly().GetType("LevelEditor.DomNodeAdapters.Locator");
                MethodInfo locType_createMI = locType.GetMethod("Create", BindingFlags.Public | BindingFlags.Static);
                var locator = locType_createMI.Invoke(null, new object[0]);
                
                Type resrefType = Assembly.GetEntryAssembly().GetType("LevelEditor.DomNodeAdapters.ResourceReference");
                MethodInfo resrefType_createMI = resrefType.GetMethod("Create", new Type[] { typeof(IResource) }, null);

                IReference<IResource> resRef = resrefType_createMI.Invoke(null, new object[] { resource }) as IReference<IResource>;

                PropertyInfo referencePI = locType.GetMember("Reference")[0] as PropertyInfo;
                referencePI.SetValue(locator, resRef, null);
                PropertyInfo domnodePI = locType.GetMember("DomNode")[0] as PropertyInfo;
                DomNode node = domnodePI.GetValue(locator, null) as DomNode;
                node.InitializeExtensions();

                gob = locator as IGameObject;
                */

                GameObject adapter = new DomNode(
                    Schema.standardBaseType.Type
                ).As<GameObject>();
                adapter.DomNode.InitializeExtensions();
                gob = adapter;                
            }
            return gob;
        }
    }
}
