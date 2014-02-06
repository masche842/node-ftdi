{
  'targets': [
    {
      'target_name': 'ftdi',
      'sources':
      [
        'src/ftdi_device.cc',
	      'src/ftdi_driver.cc'
      ],
      'include_dirs+':
      [
        'src/',
      ],
      'conditions':
      [
        ['OS == "win"',
          {
            'include_dirs+':
            [
              'lib/'
            ],
            'link_settings':
            {
              "conditions" :
              [
                ["target_arch=='ia32'",
                {
                  'libraries':
                  [
                   '-llib/i386/ftd2xx.lib'
                  ]
                }
              ],
              ["target_arch=='x64'", {
                'libraries': [
                   '-llib/amd64/ftd2xx.lib'
                ]
              }]
            ]
          }
        }],
        ['OS != "win"',
          {
            'include_dirs+': [
              '/usr/local/include/libftd2xx/'
            ],
            'ldflags': [
              '-Wl,-Map=output.map'
            ],
            'link_settings': {
              'libraries': [
                '-lftd2xx',
                '-ljansson'
              ]
            }
          }
        ]
      ],
    },
     {
        'target_name': 'fake_libftd2xx',
        'type': 'static_library',
        'sources': [ 'mocks/FakeFtd2xx.c' ],
        'include_dirs+': [ 'lib' ]
      }
  ]
}
