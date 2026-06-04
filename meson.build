project('rasptui', 'c',
  version : '0.1.0',
  default_options : [
    'warning_level=0',
    'c_std=c11',
    'werror=false',
    'buildtype=release',
    'optimization=s',
    'b_lto=true',
    'b_ndebug=true',
    'b_asneeded=true',
    'b_pgo=off'
  ]
)

# 1. 컴파일러 다이어트 플래그
add_project_arguments(
  '-D_DEFAULT_SOURCE', 
  '-D_POSIX_C_SOURCE=200809L',
  '-ffunction-sections', 
  '-fdata-sections',
  '-fno-asynchronous-unwind-tables', 
  language : 'c'
)

# 2. 링크 단계 섹션 제거 플래그
add_project_link_arguments(
  '-Wl,--gc-sections',
  '-Wl,--strip-all',
  language : 'c'
)

# 3. 의존성 정의
ncurses_dep = dependency('ncurses', required : true)
dependencies = [ncurses_dep]

# 4. 소스 파일 및 헤더 디렉토리 자동 검색
find_src = run_command('sh', '-c', 'find . -path "./bin" -prune -o -name "*.c" -print', check : true)
src_files = find_src.stdout().strip().split('\n')

find_dirs = run_command('sh', '-c', 'find . -path "./bin" -prune -o -name "*.h" -exec dirname {} \\; | sort -u', check : true)
raw_dirs = find_dirs.stdout().strip().split('\n')

inc_dirs = []
foreach d : raw_dirs
  if d != ''
    inc_dirs += include_directories(d)
  endif
endforeach

# 5. 실행 파일 생성 (변수에 할당)
exe = executable('rasptui',
  src_files,
  dependencies : dependencies,
  include_directories : inc_dirs,
  install : true
)


# 빌드 디렉토리 안의 파일 경로를 명확히 잡기 위해 빌드 타겟의 풀 경로를 가져옵니다.
exe_path = exe.full_path()

# 컴파일 완료 후 터미널에 Makefile과 똑같은 연출을 하는 커스텀 타겟 정의
run_target('atp_msg',
  command : [
    'sh', '-c',
    '''
    echo "=========================================="
    echo "배포용 바이너리 생성 완료: rasptui"
    size @0@ | tee build_size.log
    echo "=========================================="
    '''.format(exe_path)
  ],
  depends : exe
)
