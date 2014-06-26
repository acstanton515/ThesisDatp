# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('datp', ['core', 'aodv', 'internet', 'config-store', 'tools'])
    module.source = [
        'model/datp-aggregator.cc',
        'model/datp-application.cc',
        'model/datp-collector.cc',
        'model/datp-function.cc',
        'model/datp-function-simple.cc',
        'model/datp-headers.cc',
        'model/datp-scheduler.cc',
        'model/datp-scheduler-simple.cc',
        'model/datp-tree-controller.cc',
        'model/datp-tree-controller-aodv.cc',
        'helper/datp-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('datp')
    module_test.source = [
        'test/datp-test-suite.cc',
        ]

    headers = bld.new_task_gen(features=['ns3header'])
    headers.module = 'datp'
    headers.source = [
        'model/datp-aggregator.h',
        'model/datp-application.h',
        'model/datp-collector.h',
        'model/datp-function.h',
        'model/datp-function-simple.h',
        'model/datp-headers.h',
        'model/datp-scheduler.h',
        'model/datp-scheduler-simple.h',
        'model/datp-tree-controller.h',
        'model/datp-tree-controller-aodv.h',
        'helper/datp-helper.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.add_subdirs('examples')

    # bld.ns3_python_bindings()

